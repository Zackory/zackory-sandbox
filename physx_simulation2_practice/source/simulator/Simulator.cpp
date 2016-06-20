//
//  Simulator.cpp
//  physx_test
//
//  Created by YuWenhao on 10/6/15.
//  Copyright (c) 2015 YuWenhao. All rights reserved.
//

#include "Simulator.h"
#include <iostream>
#include <unistd.h>

#include "config.h"

using namespace std;
using namespace Eigen;
using namespace physx;

void Simulator::initialize() {
    baseSimulator::initialize();
    
    move_step[0] = - moving_speed * gTimeStep;
    
    // create capsule
    // forearm
    
    rig_parts.resize(2);
    PxMaterial* material = gPhysicsSDK->createMaterial(1.0f,1.0f,0.5f);
    PxRigidStatic* new_rigid = NULL;
    while (new_rigid == NULL) {
        new_rigid = gPhysicsSDK->createRigidStatic(PxTransform(PxVec3(0)));
    }
    rig_parts[0].components.push_back(new_rigid);
    PxShape* aCapsuleShape = rig_parts[0].components[0]->createShape(PxCapsuleGeometry(capsule_radius, forearm_length),*material);
    aCapsuleShape->setLocalPose(PxTransform(PxQuat(0, 0, 1, 0)));
    rig_parts[0].ft_center = PxVec3(-forearm_length-capsule_radius, 0, 0);
    rig_parts[0].descriptors.push_back(new CapsuleDescriptor(forearm_length, capsule_radius));
    
    rig_parts[0].rotate(0, -0.0116661374, PxVec3(0, 0, 1), true);
    rig_parts[0].translate(0, PxVec3(-0.1, capsule_height, 0), true);   // move to the left to avoid collision with cloth initially
    
    // create gripper
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
    rig_parts[1].translate(0, PxVec3(forearm_length + capsule_radius +0.06, 0.2, 0), true);
    rig_parts[1].translate(1, PxVec3(forearm_length + capsule_radius + 0.06, 0.06, 0));
    
    rig_parts[0].addToScene(gScene);
    rig_parts[0].recordInitTransform();
    rig_parts[1].addToScene(gScene);
    rig_parts[1].recordInitTransform();
    
    gScene->setGravity(PxVec3(0));
    
    recorded_positiosn.clear();
    recorded_time.clear();
    simulate_time = 0;
    
    // create cloth
    PxTransform gPose = PxTransform(PxVec3(0,0,0));
    Eigen::Matrix3d rot;
    rot = Eigen::AngleAxisd(1.57, Eigen::Vector3d::UnitZ()) * Eigen::AngleAxisd(0, Eigen::Vector3d::UnitX()) * Eigen::AngleAxisd(0, Eigen::Vector3d::UnitY());
    cloth.loadMesh(string(PHYSX_ROOT_PATH)+"/Mesh/sleeve.obj", Vector3d(0.405, 0.06, 0), rot, Vector3d(1,1,1));
    //cloth.loadMesh("Mesh/turtleneck.obj");
    cloth.createCloth(gPhysicsSDK, gPose);
    
    cloth.vstretch_stiff[0] = 0.350458;
    cloth.hstretch_stiff[0] = 0.350458;
    cloth.bend_stiff[0] = 0.413354;
    cloth.shear_stiff[0] = 0.413354;
    cloth.friction = 0.941999;
    cloth.self_friction = 0.126061;
    cloth.self_collision_distance = 0.0468455;
    cloth.damping[0] = 0.03399712;
    cloth.damping[1] = 0.03399712;
    cloth.damping[2] = 0.03399712;
    cloth.stiffpower = (unsigned int)4;
    cloth.setParameters();
    
    gScene->addActor(*cloth.mCloth);
    
    cout << "using iteration number: " << cloth_solve_iteration_aftergrip << endl;
    cout << "using stiff power: " << cloth.stiffpower << endl;
    cout << "initialize ok" << endl;
    
}


// reset to initial state
void Simulator::reset() {
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
    
    if (sim_stage == 2) {
        //gScene->removeActor(*rig_parts[0].components[0]);
    }
    
    sim_stage = 0;
    gScene->setGravity(PxVec3(0));
    rig_parts[1].gripper_constraints.clear();
    rig_parts[1].gripper_previous.clear();
    
    for (int i = 0; i < rig_parts.size(); i++) {
        rig_parts[i].reset();
    }
    
    cloth_solve_iteration = 1;
    
    /*cloth.mCloth->setFrictionCoefficient(cloth.mCloth->getFrictionCoefficient()+0.1);
    cout << "current friction coefficient: " << cloth.mCloth->getFrictionCoefficient() << endl;*/
    /*cloth.shear_stiff[0] -= 0.1;
    cloth.bend_stiff[0] -= 0.1;
    cloth.mCloth->setStretchConfig(PxClothFabricPhaseType::eSHEARING,PxClothStretchConfig(cloth.shear_stiff[0], cloth.shear_stiff[1], cloth.shear_stiff[2], cloth.shear_stiff[3]));
    cloth.mCloth->setStretchConfig(PxClothFabricPhaseType::eBENDING, PxClothStretchConfig(cloth.bend_stiff[0], cloth.bend_stiff[1], cloth.bend_stiff[2], cloth.bend_stiff[3]));
    cout << "shear: " << cloth.shear_stiff[0] << endl;
    cout << "bend: " << cloth.bend_stiff[0] << endl;*/
    /*cloth.vstretch_stiff[0] -= 0.1;
    cloth.hstretch_stiff[0] -= 0.1;
    cloth.mCloth->setStretchConfig(PxClothFabricPhaseType::eVERTICAL,PxClothStretchConfig(cloth.vstretch_stiff[0], cloth.vstretch_stiff[1], cloth.vstretch_stiff[2], cloth.vstretch_stiff[3]));
    cloth.mCloth->setStretchConfig(PxClothFabricPhaseType::eHORIZONTAL, PxClothStretchConfig(cloth.hstretch_stiff[0], cloth.hstretch_stiff[1], cloth.hstretch_stiff[2], cloth.hstretch_stiff[3]));
    cout << "vstr: " << cloth.vstretch_stiff[0] << endl;
    cout << "hstr: " << cloth.hstretch_stiff[0] << endl;*/
    /*cloth.damping[0] += 0.02;
    cloth.damping[1] += 0.02;
    cloth.damping[1] += 0.02;
    cloth.mCloth->setDampingCoefficient(PxVec3(cloth.damping[0], cloth.damping[1], cloth.damping[2]));
    */
    //cloth.mCloth->setSelfCollisionDistance(cloth.mCloth->getSelfCollisionDistance() + 0.002f);
    
    //cloth.mCloth->setSelfCollisionStiffness(cloth.mCloth->getSelfCollisionStiffness() + 0.1);
    //cout << "self collision distance: " << cloth.mCloth->getSelfCollisionDistance() << endl;
    //cout << "self collision stiffness: " << cloth.mCloth->getSelfCollisionStiffness() << endl;
    
    //cloth.mCloth->setSelfFrictionCoefficient(cloth.mCloth->getSelfFrictionCoefficient() + 0.1f);
    //cout << "self collision friction: " << cloth.mCloth->getSelfFrictionCoefficient() << endl;
    
    //cloth_solve_iteration_aftergrip += 5;
    
    cloth.damping[0] = 0.03399712;
    cloth.damping[1] = 0.03399712;
    cloth.damping[2] = 0.03399712;
    cloth.mCloth->setDampingCoefficient(PxVec3(cloth.damping[0], cloth.damping[1], cloth.damping[2]));
    
    recorded_positiosn.clear();
    recorded_time.clear();
    simulate_time = 0;
    simulated_step = 0;
}

int Simulator::getParameterSize() {
    return cloth.parameter_size + parameter_size;
}

void Simulator::getParameterBound(VectorXd& min, VectorXd& max) {
    VectorXd cloth_min, cloth_max;
    cloth.getParameterBound(cloth_min, cloth_max);
    min.resize(getParameterSize());
    max.resize(getParameterSize());
    min.segment(0, cloth.parameter_size) = cloth_min;
    max.segment(0, cloth.parameter_size) = cloth_max;
    min(cloth.parameter_size) = 1;
    max(cloth.parameter_size) = 40;
}

// simulate for certain steps
void Simulator::simulate(int steps) {
    if (sim_stage == 0) {   // gripper closing
        PxVec3 dif = -rig_parts[1].components[1]->getGlobalPose().p + rig_parts[1].components[0]->getGlobalPose().p;
        double thickness = ((BoxDescriptor*)rig_parts[1].descriptors[0])->half_height * 2;
        if (dif.magnitude() > thickness) {
            rig_parts[1].translate(1, dif / 60);
        } else {
            sim_stage = 1;
            gScene->setGravity(PxVec3(0, -9.8, 0));
            identifyGrippedParticles();
            
            double randx = (rand()*1.0/RAND_MAX-0.5)/3;
            double randy = (rand()*1.0/RAND_MAX-0.5)/3;
            double randz = (rand()*1.0/RAND_MAX-0.5)/3;
            random_wind = PxVec3(randx, randy, randz);
            //cout << randx << " " << randy << " " << randz << endl;
        }
    } else if (sim_stage == 1 && simulated_step > 500) {
        sim_stage = 2;
        //rig_parts[0].addToScene(gScene);
        cloth.damping[0] = 0.0002721096;
        cloth.damping[1] = 0.0002721096;
        cloth.damping[2] = 0.0002721096;
        cloth.mCloth->setDampingCoefficient(PxVec3(cloth.damping[0], cloth.damping[1], cloth.damping[2]));
    } else if (sim_stage == 1 && simulated_step < 200) {
        PxClothParticleData* data = cloth.mCloth->lockParticleData(PxDataAccessFlag::eWRITABLE);
        float dt = cloth.mCloth->getPreviousTimeStep();
        for(PxU32 i = 0, n = cloth.mCloth->getNbParticles(); i < n; ++i)
        {
            //data->previousParticles[i].pos -= random_wind * dt;
        }
        data->unlock();
    } else if (sim_stage == 1 && simulated_step >= 200 && simulated_step < 400) {
        // move arm to the right place
        rig_parts[0].translate(0, PxVec3(0.1/(400-200), 0, 0), true);
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
            cloth.mCloth->setSolverFlags(64+32+16+8+4+2);
            gScene->simulate(gTimeStep);	//Advances the simulation by 'gTimeStep' time
            gScene->fetchResults(true);
        }
        
        /*cloth.mCloth->setSolverFlags(16+8+4+2+1);
        gScene->simulate(gTimeStep);	//Advances the simulation by 'gTimeStep' time
        gScene->fetchResults(true);
        for (int j = 0; j < cloth_solve_iteration - 1; j++) {
            cloth.mCloth->setSolverFlags(16+8);
            gScene->simulate(gTimeStep);	//Advances the simulation by 'gTimeStep' time
            gScene->fetchResults(true);
        }
        cloth.mCloth->setSolverFlags(64+32);
        for (int j = 0; j < 5; j++) {
            gScene->simulate(gTimeStep);
            gScene->fetchResults(true);
        }*/
        
        simulate_time += gTimeStep;
        updateForces(steps);
    }
    cloth.updateMesh();
    double total_mass = 0;
    for (int i = 0; i < cloth.cloth_mesh.particles.size(); i++) {
        total_mass += cloth.cloth_mesh.particles[i].mass;
    }
    
    for (int j = 0; j < rig_parts.size(); j++) {
        for (int i = 0; i < rig_parts[j].gripper_constraints.size(); i++) {
            total_mass -= cloth.cloth_mesh.particles[rig_parts[j].gripper_constraints[i].first].mass * (grip_massscale-1) / grip_massscale;
        }
    }
    if (sim_stage == 2) {
        recorded_positiosn.push_back(rig_parts[1].components[0]->getGlobalPose().p.x);
        recorded_time.push_back(simulate_time);
        
        // zero out the mass of cloth
        rig_parts[1].total_force[1] += total_mass * 9.8;
        
        for (int i = 0; i < rig_parts.size(); i++) {
            rig_parts[i].recordForceTorque();
        }
        if (abs(rig_parts[1].total_force[0]) > 10 || abs(rig_parts[1].total_force[1]) > 10 || rig_parts[1].components[1]->getGlobalPose().p.x < forearm_length+capsule_radius+0.06-0.85) {
            simulated_step = -100;
        }
    }
    //cout << "vert force: " << rig_parts[1].total_force[1] << "  total weight: " << total_mass * 9.8 << endl;
    
    simulated_step++;
}

void Simulator::updateForces(int steps) {    // update collision forces and solve gripper constraints
    
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

void Simulator::identifyGrippedParticles() {
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
    
    cloth.mCloth->setTetherConfig(PxClothTetherConfig(0, 0));
    //cloth.mCloth->setSelfCollisionDistance(0.01f);
    //cloth.mCloth->setSelfCollisionStiffness(1.0f);
    
    cloth_solve_iteration = cloth_solve_iteration_aftergrip;
    
    gScene->removeActor(*rig_parts[1].components[0]);
    gScene->removeActor(*rig_parts[1].components[1]);
}

// set parameter
void Simulator::setParameter(VectorXd param) {
    cloth.setParameter(param.segment(0, param.size() - parameter_size));
    cloth_solve_iteration_aftergrip = param(param.size()-1);
}




