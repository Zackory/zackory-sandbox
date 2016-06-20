//
//  MatSimulator.cpp
//  physx_test
//
//  Created by YuWenhao on 1/19/16.
//  Copyright (c) 2016 YuWenhao. All rights reserved.
//

#include "MatSimulator.h"
#include <iostream>

#include <unistd.h>

#include "config.h"

using namespace std;
using namespace Eigen;
using namespace physx;

void MatSimulator::initialize() {
    baseSimulator::initialize();
    
    // create cloth
    PxTransform gPose = PxTransform(PxVec3(0,0,0));
    Matrix3d rot;
    rot = AngleAxisd(0, Vector3d::UnitZ()) * AngleAxisd(0, Vector3d::UnitX()) * AngleAxisd(0, Vector3d::UnitY());
    cloth.density = 0.245;
    cloth.hstretch_stiff[0] = cloth.vstretch_stiff[0] = 0.05;
    cloth.bend_stiff[0] = cloth.shear_stiff[0] = 0.05;
    
    //cloth.loadMesh("Mesh/tshirt_m_down.obj", Vector3d(0.405, 0.06, 0), rot, Vector3d(1,1,1));
    //cloth.loadMesh("Mesh/hankerchief.obj", Vector3d(0, 0.06, 0), rot, Vector3d(1,1,1));
    rot = Eigen::AngleAxisd(1.57, Eigen::Vector3d::UnitZ()) * Eigen::AngleAxisd(0, Eigen::Vector3d::UnitX()) * Eigen::AngleAxisd(0, Eigen::Vector3d::UnitY());
    cloth.loadMesh(string(PHYSX_ROOT_PATH)+"/Mesh/sleeve.obj", Vector3d(-0.05, 0.06, 0), rot, Vector3d(1,1,1));
    cloth.createCloth(gPhysicsSDK, gPose);
    cloth.mCloth->setStiffnessPower(8);
    
    gScene->addActor(*cloth.mCloth);
    
    cloth.mCloth->setSelfCollisionDistance(0.01f);
    cloth.mCloth->setSelfCollisionStiffness(1.0f);
    
    // create ground
    rig_parts.resize(3);
    PxRigidStatic* new_rigid = NULL;
    PxMaterial* material = gPhysicsSDK->createMaterial(1.0f,1.0f,0.5f);
    while (new_rigid == NULL) {
        new_rigid = gPhysicsSDK->createRigidStatic(PxTransform(PxVec3(0)));
    }
    rig_parts[0].components.push_back(new_rigid);
    rig_parts[0].components[0]->createShape(PxBoxGeometry(4, 0.05, 6), *material);
    rig_parts[0].descriptors.push_back(new BoxDescriptor(4, 0.05, 6));
    
    rig_parts[0].translate(0, PxVec3(0, -0.5, 0));
    rig_parts[0].addToScene(gScene);
    rig_parts[0].recordInitTransform();
    
    /*// create clipper1
    new_rigid = NULL;
    while (new_rigid == NULL) {
        new_rigid = gPhysicsSDK->createRigidStatic(PxTransform(PxVec3(0)));
    }
    rig_parts[1].components.push_back(new_rigid);
    rig_parts[1].components[0]->createShape(PxBoxGeometry(0.05, 0.005, 0.22), *material);
    rig_parts[1].descriptors.push_back(new BoxDescriptor(0.05, 0.005, 0.22));
    
    rig_parts[1].translate(0, PxVec3(0.25, 0.06, 0), true);
    rig_parts[1].addToScene(gScene);
    rig_parts[1].recordInitTransform();
    
    // create clipper2
    new_rigid = NULL;
    while (new_rigid == NULL) {
        new_rigid = gPhysicsSDK->createRigidStatic(PxTransform(PxVec3(0)));
    }
    rig_parts[2].components.push_back(new_rigid);
    rig_parts[2].components[0]->createShape(PxBoxGeometry(0.05, 0.005, 0.22), *material);
    rig_parts[2].descriptors.push_back(new BoxDescriptor(0.05, 0.005, 0.22));
    
    rig_parts[2].translate(0, PxVec3(-0.25, 0.06, 0), true);
    rig_parts[2].addToScene(gScene);
    rig_parts[2].recordInitTransform();*/
    
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
    rig_parts[1].translate(0, PxVec3(0, 0.2, 0), true);
    rig_parts[1].translate(1, PxVec3(0, 0.06, 0));
    
    rig_parts[1].addToScene(gScene);
    rig_parts[1].recordInitTransform();
    
    gScene->setGravity(PxVec3(0, 0, 0));
    
    identifyGrippedParticles();
    
    Transformation t1;
    t1.translation = Vector3d(-0.3, -0.2, 0);
    t1.time = 0.0;
    paths[0].AddKeyframe(t1);
    t1.translation = Vector3d(-0.05, -0.2, 0);
    t1.time = 3.0;
    paths[0].AddKeyframe(t1);
    t1.translation = Vector3d(-0.3, -0.2, 0);
    t1.time = 5.0;
    paths[0].AddKeyframe(t1);
    t1.translation = Vector3d(-0.3, -0.1, 0);
    t1.time = 6.0;
    paths[0].AddKeyframe(t1);
    t1.translation = Vector3d(-0.05, -0.1, 0);
    t1.time = 9.0;
    paths[0].AddKeyframe(t1);
    /*t1.translation = Vector3d(-0.25, 0.06, 0);
    t1.time = 0.0;
    paths[1].AddKeyframe(t1);
    t1.translation = Vector3d(-0.29, 0.06, 0);
    t1.time = 7.0;
    paths[1].AddKeyframe(t1);*/
    
    // add obstacle
    new_rigid = NULL;
    while (new_rigid == NULL) {
        new_rigid = gPhysicsSDK->createRigidStatic(PxTransform(PxVec3(0)));
    }
    rig_parts[2].components.push_back(new_rigid);
    rig_parts[2].components[0]->createShape(PxCapsuleGeometry(0.06, 0.2), *material);
    rig_parts[2].descriptors.push_back(new CapsuleDescriptor(0.2, 0.06));
    
    rig_parts[2].translate(0, PxVec3(-0.3, -0.2, 0));
    rig_parts[2].addToScene(gScene);
    rig_parts[2].recordInitTransform();
    
    cloth.vstretch_stiff[0] = 0.473218953173759;
    cloth.hstretch_stiff[0] = 0.473218953173759;
    cloth.bend_stiff[0] = 0.409699192271476;
    cloth.shear_stiff[0] = 0.409699192271476;
    cloth.friction = 0.8738111496639649;
    cloth.self_friction = 0.2765414915436417;
    cloth.self_collision_distance = 0.00964945;
    cloth.damping[0] = 0.0002721096;
    cloth.damping[1] = 0.0002721096;
    cloth.damping[2] = 0.0002721096;
    cloth.stiffpower = (unsigned int)4;
    cloth.setParameters();
}


// reset to initial state
void MatSimulator::reset() {
    // reset cloth mesh
    cloth.resetCloth();
    // reset physx cloth
    PxClothParticleData* particle_data = cloth.mCloth->lockParticleData(PxDataAccessFlag::eWRITABLE);
    for (int i = 0; i < cloth.cloth_mesh.particles.size(); i++) {
        PxVec3 init_pos = PxVec3(cloth.cloth_mesh.particles[i].pos(0), cloth.cloth_mesh.particles[i].pos(1), cloth.cloth_mesh.particles[i].pos(2));
        particle_data->particles[i].pos = init_pos;
        particle_data->previousParticles[i].pos = init_pos;
        if (cloth.cloth_mesh.particles[i].mass > 1000) {
            particle_data->particles[i].invWeight = 0;
            particle_data->previousParticles[i].invWeight = 0;
        } else {
            particle_data->particles[i].invWeight = 1.0/cloth.cloth_mesh.particles[i].mass;
            particle_data->previousParticles[i].invWeight = 1.0/cloth.cloth_mesh.particles[i].mass;
        }
    }
    particle_data->unlock();
    
    for (int i = 0; i < rig_parts.size(); i++) {
        rig_parts[i].gripper_constraints.clear();
        rig_parts[i].gripper_previous.clear();
    }
    
    for (int i = 0; i < rig_parts.size(); i++) {
        rig_parts[i].reset();
    }
    
    recorded_strain.clear();
    
    identifyGrippedParticles();
    
    simulated_time = 0;
    
    sim_stage = 0;
    
    gScene->setGravity(PxVec3(0, 0, 0));
}

int MatSimulator::getParameterSize() {
    return 0;
}

void MatSimulator::getParameterBound(VectorXd& min, VectorXd& max) {
    
}

// simulate for certain steps
void MatSimulator::simulate(int steps) {
    if (sim_stage == 0) {   // gripper closing
        PxVec3 dif = -rig_parts[1].components[1]->getGlobalPose().p + rig_parts[1].components[0]->getGlobalPose().p;
        double thickness = ((BoxDescriptor*)rig_parts[1].descriptors[0])->half_height * 2;
        if (dif.magnitude() > thickness) {
            rig_parts[1].translate(1, dif / 60);
        } else {
            sim_stage = 1;
            gScene->setGravity(PxVec3(0, -9.8, 0));
            identifyGrippedParticles();
        }
    }
    
    for (int i = 0; i < steps; i++) {
        // update clipper motion
        Transformation t = paths[0].getLinearInterpolatedTransformation(simulated_time, true);

        rig_parts[2].translateTo(0, PxVec3(t.translation(0), t.translation(1), t.translation(2)), true);
        /*t = paths[1].getLinearInterpolatedTransformation(simulated_time, true);
        rig_parts[2].translateTo(0, PxVec3(t.translation(0), t.translation(1), t.translation(2)), true);
        
        // set the velocities for the clipped particles
        setClippedVelocities();*/
        
        
        
        cloth.mCloth->setSolverFlags(64+32+16+8+4+2+1);
        gScene->simulate(gTimeStep);	//Advances the simulation by 'gTimeStep' time
        gScene->fetchResults(true);
        
        for (int j = 0; j < iteration_number-1; j++) {
            cloth.mCloth->setSolverFlags(64+32+16+8+4+2);
            gScene->simulate(gTimeStep);
            gScene->fetchResults(true);
        }
        
        updateForces(steps);
        
        simulated_time += steps * gTimeStep;
    }
    cloth.updateMesh();
    
    if (sim_stage != 0) {
        recorded_time.push_back(simulated_time);
        
        double total_mass = 0;
        for (int i = 0; i < cloth.cloth_mesh.particles.size(); i++) {
            total_mass += cloth.cloth_mesh.particles[i].mass;
        }
        
        for (int j = 0; j < rig_parts.size(); j++) {
            for (int i = 0; i < rig_parts[j].gripper_constraints.size(); i++) {
                total_mass -= cloth.cloth_mesh.particles[rig_parts[j].gripper_constraints[i].first].mass * (grip_massscale-1) / grip_massscale;
            }
        }
        
        // zero out the mass of cloth
        rig_parts[1].total_force[1] += total_mass * 9.8;
        
        for (int i = 0; i < rig_parts.size(); i++) {
            rig_parts[i].recordForceTorque();
        }
        
        recorded_strain.push_back((rig_parts[1].components[0]->getGlobalPose().p.x - 0.05)/0.2-1);
    }
}

void MatSimulator::setClippedVelocities() {
    PxClothParticleData* particle_data = cloth.mCloth->lockParticleData(PxDataAccessFlag::eWRITABLE);
    
    // compute grip constraint force and modify particle positions
    for (int j = 0; j < rig_parts.size(); j++) {
        for (int i = 0; i < rig_parts[j].gripper_constraints.size(); i++) {
            particle_data->previousParticles[rig_parts[j].gripper_constraints[i].first].pos = particle_data->particles[rig_parts[j].gripper_constraints[i].first].pos - (rig_parts[j].gripper_constraints[i].second - rig_parts[j].gripper_previous[i]);
        }
    }
    particle_data->unlock();
}

void MatSimulator::updateForces(int steps) {    // update collision forces and solve gripper constraints
    
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
}

void MatSimulator::identifyGrippedParticles() {
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
        }
    }
    
    /*PxBounds3 bound2 = rig_parts[2].components[0]->getWorldBounds();
    for (int i = 0; i < cloth.cloth_mesh.particles.size(); i++) {
        if (bound2.contains(particle_data->particles[i].pos)) {
            rig_parts[2].gripper_constraints.push_back(make_pair(i, particle_data->particles[i].pos));
            rig_parts[2].gripper_previous.push_back(particle_data->particles[i].pos);
            particle_data->particles[i].invWeight /= grip_massscale;
            particle_data->previousParticles[i].invWeight /= grip_massscale;
            cloth.cloth_mesh.particles[i].mass *= grip_massscale;
        }
    }*/
    
    particle_data->unlock();
}

// set parameter
void MatSimulator::setParameter(VectorXd param) {
}