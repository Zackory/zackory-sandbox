//
//  MatSimulator.h
//  physx_test
//
//  Created by YuWenhao on 1/19/16.
//  Copyright (c) 2016 YuWenhao. All rights reserved.
//

#ifndef __physx_test__MatSimulator__
#define __physx_test__MatSimulator__

#include <stdio.h>

#include "baseSimulator.h"
#include "Cloth.h"
#include "RigPart.h"
#include <PxPhysicsAPI.h>
#include <Eigen/Dense>
#include "path/TransformationPath.h"

class MatSimulator : public baseSimulator {
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
    
    void setClippedVelocities();
    
    void identifyGrippedParticles();
    
    void updateForces();
    
    float iteration_number;
    
    // mass scale for gripped particles
    double grip_massscale = 100;
    
    // path for two clippers
    TransformationPath paths[2];
    
    double simulated_time = 0;
    
    int sim_stage = 0;
    
    std::vector<double> recorded_strain;
    
    std::vector<double> recorded_time;
};

#endif /* defined(__physx_test__MatSimulator__) */
