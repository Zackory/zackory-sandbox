//
//  Simulator.h
//  physx_test
//
//  Created by YuWenhao on 10/6/15.
//  Copyright (c) 2015 YuWenhao. All rights reserved.
//

#ifndef __physx_test__Simulator__
#define __physx_test__Simulator__

#include <stdio.h>

#include "baseSimulator.h"
#include "Cloth.h"
#include "RigPart.h"
#include <PxPhysicsAPI.h>
#include <Eigen/Dense>

class Simulator : public baseSimulator {
public:
    void initialize();
        
    // reset to initial state
    void reset();
    
    // simulate for certain steps
    void simulate(int);
    
    // set parameter
    void setParameter(Eigen::VectorXd);
    
    int getParameterSize();
    
    void getParameterBound(Eigen::VectorXd&, Eigen::VectorXd&);
    
    void updateForces(int);    // update collision forces and solve gripper constraints
    
    void identifyGrippedParticles();
    
    // stage control
    int sim_stage = 0;  // 0: gripping, 1: stop, 2: moving
    physx::PxVec3 move_step = physx::PxVec3(-0.05, 0, 0);
    int simulated_step = 0;
    float simulate_time = 0;
    
    // simulated cloth
    int cloth_solve_iteration = 1;
    int cloth_solve_iteration_aftergrip = 15;
    physx::PxVec3 random_wind = physx::PxVec3(0, 0, 0);
    
    // record position and timing of force-torque data
    std::vector<double> recorded_positiosn;
    std::vector<double> recorded_time;
    
    // mass scale for gripped particles
    double grip_massscale = 100;
    
    // moving speed of linear actuator
    double moving_speed = 0.1;  // 0.1 mps
    
    // height of fake arm
    double capsule_radius = 0.060325/2;
    double forearm_length = 0.74/2.0;
    double capsule_height = 0.195 + 0.058 - capsule_radius - 0.105833333 /*0.277333333*/;
};


#endif /* defined(__physx_test__Simulator__) */
