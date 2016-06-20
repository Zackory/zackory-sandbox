//
//  TransformationPath.h
//  physx_test
//
//  Created by YuWenhao on 2/10/16.
//  Copyright (c) 2016 YuWenhao. All rights reserved.
//

#ifndef __physx_test__TransformationPath__
#define __physx_test__TransformationPath__

#include <stdio.h>

#include <Eigen/Dense>
#include <vector>

struct Transformation {
    Transformation() {
        time = 0;
        translation.setZero();
        rotation.setIdentity();
        scale.setOnes();
    }
    
    Eigen::Vector3d translation;
    Eigen::Quaterniond rotation;
    Eigen::Vector3d scale;
    
    double time;
};

class TransformationPath {
public:
    Transformation getLinearInterpolatedTransformation(double t, bool reverse = false);
    
    void AddKeyframe(Transformation);
    
private:
    void getInterpFrac(double t, int& ind1, double& frac1, int& ind2, double& frac2, bool reverse);
    
    std::vector<Transformation> keyframes;
};

#endif /* defined(__physx_test__TransformationPath__) */
