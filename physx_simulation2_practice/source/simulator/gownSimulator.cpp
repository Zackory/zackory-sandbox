//
//  gownSimulator.cpp
//  physx_test
//
//  Created by YuWenhao on 10/6/15.
//  Copyright (c) 2015 YuWenhao. All rights reserved.
//

#include "gownSimulator.h"
#include <iostream>

#include "myUtils/mathlib.h"

#include <unistd.h>
#include "config.h"

using namespace std;
using namespace Eigen;
using namespace physx;

void gownSimulator::initialize() {
    baseSimulator::initialize();
    
    move_step[0] = - moving_speed * gTimeStep;
    
    // create capsule
    rig_parts.resize(2);
    
    // create gripper
    PxMaterial* material = gPhysicsSDK->createMaterial(1.0f,1.0f,0.5f);
    PxRigidStatic* new_rigid = NULL;
    new_rigid = NULL;
    while (new_rigid == NULL) {
        new_rigid = gPhysicsSDK->createRigidStatic(PxTransform(PxVec3(0)));
    }
    rig_parts[1].components.push_back(new_rigid);
    new_rigid = NULL;
    while (new_rigid == NULL) {
        new_rigid = gPhysicsSDK->createRigidStatic(PxTransform(PxVec3(0)));
    }
    rig_parts[1].components.push_back(new_rigid);
    rig_parts[1].components[0]->createShape(PxBoxGeometry(0.04, 0.005, 0.06), *material);
    rig_parts[1].components[1]->createShape(PxBoxGeometry(0.04, 0.005, 0.06), *material);
    rig_parts[1].descriptors.push_back(new BoxDescriptor(0.04, 0.005, 0.06));
    rig_parts[1].descriptors.push_back(new BoxDescriptor(0.04, 0.005, 0.06));
    rig_parts[1].ft_center = PxVec3(0, 0.005, 0);
    rig_parts[1].translate(0, PxVec3(-forearm_length+0.48, 0.2, 0), true);
    rig_parts[1].translate(1, PxVec3(-forearm_length+0.48, 0.06, 0));
    
    rig_parts[1].addToScene(gScene);
    rig_parts[1].recordInitTransform();
    
    // create arm
    PxSphereManager* sphereManager = PxSphereManager::GetSingleton();
    sphereManager->addSphere(PxVec3(-fist_radius-0.1+arm_horizontal_perturb, arm_height, 0), fist_radius); // fist
    sphereManager->addSphere(PxVec3(-fist_radius-0.1+arm_horizontal_perturb, arm_height, 0), wrist_radius); // wrist
    sphereManager->addSphere(PxVec3(-forearm_length-elbow_radius-0.1+arm_horizontal_perturb, arm_height, 0), elbow_radius); // elbow
    sphereManager->addSphere(PxVec3(-forearm_length-elbow_radius-0.1+arm_horizontal_perturb, arm_height, -upperearm_length), shoulder_radius); // shoulder
    sphereManager->addCapsule(1, 2);
    sphereManager->addCapsule(2, 3);
    
    gScene->setGravity(PxVec3(0));
    
    recorded_positiosn.clear();
    recorded_time.clear();
    simulate_time = 0;
    
    // create cloth
    PxTransform gPose = PxTransform(PxVec3(0,0,0));
    Matrix3d rot;
    rot = AngleAxisd(1.57, Vector3d::UnitY()) * AngleAxisd(-1.2, Vector3d::UnitX()) * AngleAxisd(-3.14, Vector3d::UnitZ());
    cloth.density = 0.145;
    cloth.loadMesh(string(PHYSX_ROOT_PATH)+"/Mesh/fullgown.obj", Vector3d(-forearm_length + 0.71, -0.0, -0.3), rot, Vector3d(1.679, 1.679, 1.679));
    cloth.createCloth(gPhysicsSDK, gPose);
    
    cloth.vstretch_stiff[0] = 0.0883887326743833;
    cloth.hstretch_stiff[0] = 0.0883887326743833;
    cloth.shear_stiff[0] = 0.5869794205273706;
    cloth.bend_stiff[0] = 0.7828958548983251;
    cloth.friction = 0.5633855129488963;
    cloth.self_friction = 0.2199003909829521;
    cloth.self_collision_distance = 0.008;
    cloth.damping[0] = 0.03;
    cloth.damping[1] = 0.03;
    cloth.damping[2] = 0.03;
    cloth.stiffpower = (unsigned int)7;
    cloth.setParameters();
    
    gScene->addActor(*cloth.mCloth);
    
    sphereManager->recordInitSpheres();
    sphereManager->setUpCapsules(cloth.mCloth);
    
    cout << "using stretch: " << cloth.vstretch_stiff[0] << endl;
    cout << "using friction: " << cloth.friction << endl;
    cout << "using iteration number: " << cloth_solve_iteration_aftergrip << endl;
    cout << "using stiff power: " << cloth.stiffpower << endl;
    cout << "initialize ok" << endl;
    cout << "using damping: " << cloth.damping[0] << endl;
    cout << "using self_collision_distance: " << cloth.self_collision_distance << endl;
}


// reset to initial state
void gownSimulator::reset() {
    cout << "resetting\n";
    // reset cloth mesh
    cloth.resetCloth();
    // reset physx cloth
    PxClothParticleData* particle_data = cloth.mCloth->lockParticleData(PxDataAccessFlag::eWRITABLE);
    for (int i = 0; i < cloth.cloth_mesh.particles.size(); i++) {
        PxVec3 init_pos = PxVec3(cloth.cloth_mesh.particles[i].pos(0), cloth.cloth_mesh.particles[i].pos(1), cloth.cloth_mesh.particles[i].pos(2));
        particle_data->particles[i].pos = init_pos;
        particle_data->particles[i].invWeight = 1.0/cloth.cloth_mesh.particles[i].mass;
        particle_data->previousParticles[i].pos = init_pos;
        particle_data->previousParticles[i].invWeight = 1.0/cloth.cloth_mesh.particles[i].mass;
    }
    particle_data->unlock();
    
    if (sim_stage != 0) {   // add back the gripper collision
        gScene->addActor(*rig_parts[1].components[0]);
        gScene->addActor(*rig_parts[1].components[1]);
    }
    
    sim_stage = 0;
    gScene->setGravity(PxVec3(0));
    rig_parts[1].gripper_constraints.clear();
    rig_parts[1].gripper_previous.clear();
    
    for (int i = 0; i < rig_parts.size(); i++) {
        rig_parts[i].reset();
    }
    PxSphereManager::GetSingleton()->resetPos();
    PxSphereManager::GetSingleton()->updateCapsules(cloth.mCloth);
    
    cloth_solve_iteration = 1;
    
    cloth.damping[0] = 0.03;
    cloth.damping[1] = 0.03;
    cloth.damping[2] = 0.03;
    cloth.mCloth->setDampingCoefficient(PxVec3(cloth.damping[0], cloth.damping[1], cloth.damping[2]));
    
    recorded_positiosn.clear();
    recorded_time.clear();
    simulate_time = 0;
    simulated_step = 0;
    
    /*PxSphereManager::GetSingleton()->resetPos();
    // reset the arm with randomized size
    PxSphereManager::GetSingleton()->clearSpheres();
    arm_height_perturb = RandDouble(range_arm_height_perturb[0], range_arm_height_perturb[1]);
    //arm_horizontal_perturb = RandDouble(range_arm_horizontal_perturb[0], range_arm_horizontal_perturb[1]);
    fist_radius = RandDouble(range_fist_radius[0], range_fist_radius[1]);
    wrist_radius = RandDouble(range_wrist_radius[0], range_wrist_radius[1]);
    elbow_radius = RandDouble(range_elbow_radius[0], range_elbow_radius[1]);
    shoulder_radius = RandDouble(range_shoulder_radius[0], range_shoulder_radius[1]);
    upperearm_length = RandDouble(range_upperearm_length[0], range_upperearm_length[1]);
    forearm_length = RandDouble(range_forearm_length[0], range_forearm_length[1]);
    cloth.friction = RandDouble(range_friction[0], range_friction[1]);
    cloth.setParameters();
    
    arm_height = -0.13 + wrist_radius + 0.205 + arm_height_perturb;
    
    PxSphereManager* sphereManager = PxSphereManager::GetSingleton();
    sphereManager->addSphere(PxVec3(-fist_radius-0.1+arm_horizontal_perturb, arm_height, 0), fist_radius); // fist
    sphereManager->addSphere(PxVec3(-fist_radius-0.1+arm_horizontal_perturb, arm_height, 0), wrist_radius); // wrist
    sphereManager->addSphere(PxVec3(-forearm_length-elbow_radius-0.1+arm_horizontal_perturb, arm_height, 0), elbow_radius); // elbow
    sphereManager->addSphere(PxVec3(-forearm_length-elbow_radius-0.1+arm_horizontal_perturb, arm_height, -upperearm_length), shoulder_radius); // shoulder*/
}

int gownSimulator::getParameterSize() {
    return cloth.parameter_size + parameter_size;
}

void gownSimulator::getParameterBound(VectorXd& min, VectorXd& max) {
    VectorXd cloth_min, cloth_max;
    cloth.getParameterBound(cloth_min, cloth_max);
    min.resize(getParameterSize());
    max.resize(getParameterSize());
    min.segment(0, cloth.parameter_size) = cloth_min;
    max.segment(0, cloth.parameter_size) = cloth_max;
}

// simulate for certain steps
void gownSimulator::simulate(int steps) {
    if (sim_stage == 0) {   // gripper closing
        PxVec3 dif = -rig_parts[1].components[1]->getGlobalPose().p + rig_parts[1].components[0]->getGlobalPose().p;
        double thickness = ((BoxDescriptor*)rig_parts[1].descriptors[0])->half_height * 2;
        if (dif.magnitude() > thickness) {
            rig_parts[1].translate(1, dif / 60);
        } else {
            sim_stage = 1;
            gScene->setGravity(PxVec3(0, -9.8, 0));
            identifyGrippedParticles();
            
            double randx = (rand()*1.0/RAND_MAX-0.5)/6;
            double randy = (rand()*1.0/RAND_MAX-0.5)/6;
            double randz = 0;
            random_wind = PxVec3(randx, randy, randz);
            //cout << randx << " " << randy << " " << randz << endl;
        }
    } else if (sim_stage == 1 && simulated_step > 500) {
        sim_stage = 2;
        cloth.damping[0] = damping_aftergrip;
        cloth.damping[1] = damping_aftergrip;
        cloth.damping[2] = damping_aftergrip;
        cloth.mCloth->setDampingCoefficient(PxVec3(cloth.damping[0], cloth.damping[1], cloth.damping[2]));
    } else if (sim_stage == 1 && simulated_step < 200) {
        PxClothParticleData* data = cloth.mCloth->lockParticleData(PxDataAccessFlag::eWRITABLE);
        float dt = cloth.mCloth->getPreviousTimeStep();
        for(PxU32 i = 0, n = cloth.mCloth->getNbParticles(); i < n; ++i)
        {
            data->previousParticles[i].pos -= random_wind * dt;
        }
        data->unlock();
    } else if (sim_stage == 1 && simulated_step >= 200 && simulated_step < 400) {
        // move arm to the right place
        PxSphereManager* sphereManager = PxSphereManager::GetSingleton();
        for (int s = 0; s < sphereManager->getNumSphere(); s++) {
            sphereManager->translate(s, PxVec3(0.1/(400-200), 0, 0));
        }
        sphereManager->updateCapsules(cloth.mCloth);
    }

    for (int i = 0; i < steps; i++) {
        if (sim_stage == 2 /*&& simulated_step < 1200*/) { // gripper moving
            rig_parts[1].translate(0, move_step);
            rig_parts[1].translate(1, move_step, true);
        }
        
        cloth.mCloth->setSolverFlags(64+32+16+8+4+2+1);
        gScene->simulate(gTimeStep);	//Advances the simulation by 'gTimeStep' time
        gScene->fetchResults(true);
        for (int j = 0; j < cloth_solve_iteration - 1; j++) {
            cloth.mCloth->setSolverFlags(32+16+8);
            gScene->simulate(gTimeStep);	//Advances the simulation by 'gTimeStep' time
            gScene->fetchResults(true);
        }
        // put collision step as the last
        /*cloth.mCloth->setSolverFlags(32);
        gScene->simulate(gTimeStep);	//Advances the simulation by 'gTimeStep' time
        gScene->fetchResults(true);*/
        
        simulate_time += gTimeStep;
        updateForces(steps);
    }
    cloth.updateMesh();
    
    if (cloth.max_stretch > 5) {
        //cout << "Invalid deformation has occurred\n";
        //simulated_step = -100;
        //return;
    }
    
    double total_mass = 0;
    for (int i = 0; i < cloth.cloth_mesh.particles.size(); i++) {
        total_mass += cloth.cloth_mesh.particles[i].mass;
    }
    
    for (int j = 0; j < rig_parts.size(); j++) {
        for (int i = 0; i < rig_parts[j].gripper_constraints.size(); i++) {
            total_mass -= cloth.cloth_mesh.particles[rig_parts[j].gripper_constraints[i].first].mass * (grip_massscale-1) / grip_massscale;
        }
    }
    if (sim_stage == 2 && rig_parts[1].components[1]->getGlobalPose().p.x-0.04 < 0) {
        
        // mimic equilibrium control
        //rig_parts[0].rotateAround(0, -0.52333333333/700, PxVec3(0, 0, 1), PxVec3(0.1325-2*forearm_length-capsule_radius, capsule_height, 0));
        //rig_parts[0].rotateAround(rig_parts[0].components.size()-1, -0.52333333333/700, PxVec3(0, 0, 1), PxVec3(0.1325-2*forearm_length-capsule_radius, capsule_height, 0));
        
        recorded_positiosn.push_back(rig_parts[1].components[0]->getGlobalPose().p.x);
        recorded_time.push_back(simulate_time);
        
        // zero out the mass of cloth
        rig_parts[1].total_force[1] += total_mass * 9.8;
        
        for (int i = 0; i < rig_parts.size(); i++) {
            rig_parts[i].recordForceTorque();
        }
        if (abs(rig_parts[1].total_force[0]) > 10 || abs(rig_parts[1].total_force[1]) > 10 || rig_parts[1].components[1]->getGlobalPose().p.x-0.04 < -forearm_length+0.44-0.85) {
            simulated_step = -100;
        }
        //cout << "moved distance: " << forearm_length - rig_parts[1].components[1]->getGlobalPose().p.x << endl;
    }
    //cout << "vert force: " << rig_parts[1].total_force[1] << "  total weight: " << total_mass * 9.8 << endl;
    
    simulated_step++;
}

void gownSimulator::updateForces(int steps) {    // update collision forces and solve gripper constraints
    
    // update force
    for (int i = 0; i < rig_parts.size(); i++) {
        rig_parts[i].clearForceTorque();
    }
    float* contact_forces = cloth.mCloth->getContactForces();
    float* friction_forces = cloth.mCloth->getFrictionForces();
    PxClothParticleData* particle_data = cloth.mCloth->lockParticleData(PxDataAccessFlag::eWRITABLE);
    
    // compute grip constraint force and modify particle positions
    for (int j = 0; j < rig_parts.size(); j++) {
        for (int i = 0; i < rig_parts[j].gripper_constraints.size(); i++) {
            PxVec3 change = rig_parts[j].gripper_constraints[i].second - particle_data->particles[rig_parts[j].gripper_constraints[i].first].pos;
            
            particle_data->particles[rig_parts[j].gripper_constraints[i].first].pos += change;
            
            change += particle_data->previousParticles[rig_parts[j].gripper_constraints[i].first].pos - rig_parts[j].gripper_previous[i];
            
            // add gravity compensation for mass scaled particles
            double iterDt = 1.0/cloth.mCloth->getSolverFrequency();
            change += (grip_massscale-1.0)/(grip_massscale) * (gScene->getGravity() * iterDt * iterDt);
            
            particle_data->previousParticles[rig_parts[j].gripper_constraints[i].first].pos = rig_parts[j].gripper_previous[i];
            
            friction_forces[rig_parts[j].gripper_constraints[i].first * 3] += change[0];
            friction_forces[rig_parts[j].gripper_constraints[i].first * 3+1] += change[1];
            friction_forces[rig_parts[j].gripper_constraints[i].first * 3+2] += change[2];
        }
    }
    
    // calculate forces
    for (int i = 0; i < cloth.mCloth->getNbParticles(); i++) {
        if (particle_data->particles[i].invWeight == 0) {
            continue;
        }
        PxVec3 total_force(contact_forces[i*3] + friction_forces[i*3], contact_forces[i*3+1] + friction_forces[i*3+1], contact_forces[i*3+2] + friction_forces[i*3+2]);
        
        total_force *= cloth.mCloth->getSolverFrequency() * 1/gTimeStep;
        total_force *= cloth.cloth_mesh.particles[i].mass;
        for (int j = 0; j < rig_parts.size(); j++) {
            rig_parts[j].updateContactForce(particle_data->particles[i].pos, total_force);
        }
    }
    particle_data->unlock();
    
    //cout << "eee\n";
    //cout << rig_parts[1].total_force.x << endl;
    //cout << rig_parts[1].total_torque.x << " " << rig_parts[1].total_torque.y << " " << rig_parts[1].total_torque.z << endl;*/
}

void gownSimulator::identifyGrippedParticles() {
    PxClothParticleData* particle_data = cloth.mCloth->lockParticleData(PxDataAccessFlag::eWRITABLE);
    PxBounds3 bound1 = rig_parts[1].components[0]->getWorldBounds();
    PxBounds3 bound2 = rig_parts[1].components[1]->getWorldBounds();
    for (int i = 0; i < cloth.cloth_mesh.particles.size(); i++) {
        if (bound1.contains(particle_data->particles[i].pos) && bound2.contains(particle_data->particles[i].pos)) {
            rig_parts[1].gripper_constraints.push_back(make_pair(i, particle_data->particles[i].pos));
            rig_parts[1].gripper_previous.push_back(particle_data->particles[i].pos);
            particle_data->particles[i].invWeight /= grip_massscale;
            particle_data->previousParticles[i].invWeight /= grip_massscale;
            cloth.cloth_mesh.particles[i].mass *= grip_massscale;
            //cout << i << ", ";
        }
    }
    //cout << endl;
    
    //rig_parts[1].gripper_constraints.push_back(make_pair(0, particle_data->particles[0].pos));
    
    particle_data->unlock();
    
    cloth.mCloth->setTetherConfig(PxClothTetherConfig(1, 1));
    //cloth.mCloth->setSelfCollisionDistance(0.01f);
    //cloth.mCloth->setSelfCollisionStiffness(1.0f);
    
    cloth_solve_iteration = cloth_solve_iteration_aftergrip;
    
    gScene->removeActor(*rig_parts[1].components[0]);
    gScene->removeActor(*rig_parts[1].components[1]);
}

// set parameter
void gownSimulator::setParameter(VectorXd param) {
    cloth.setParameter(param.segment(0, param.size() - parameter_size));
    //cloth_solve_iteration_aftergrip = param(param.size()-1);
}




