//
//  TransformationPath.cpp
//  physx_test
//
//  Created by YuWenhao on 2/10/16.
//  Copyright (c) 2016 YuWenhao. All rights reserved.
//

#include "TransformationPath.h"
#include <iostream>

using namespace Eigen;
using namespace std;

Transformation TransformationPath::getLinearInterpolatedTransformation(double t, bool reverse) {
    if (keyframes.size() == 0) {
        
        cerr << "No keyframe found" << endl;
        return Transformation();
    }
    
    int ind1, ind2;
    double frac1, frac2;
    getInterpFrac(t, ind1, frac1, ind2, frac2, reverse);
    Transformation newtrans;
    // TODO: add interpolation for rotation
    newtrans.translation = frac1 * keyframes[ind1].translation + frac2 * keyframes[ind2].translation;
    newtrans.scale = frac1 * keyframes[ind1].scale + frac2 * keyframes[ind2].scale;
    return newtrans;
}

void TransformationPath::AddKeyframe(Transformation t) {
    if (keyframes.size() != 0 && t.time < keyframes[keyframes.size()-1].time) {
        cerr << "Keyframe inserted must have larger time stamp than the last one." << endl;
        return;
    }
    keyframes.push_back(t);
}


void TransformationPath::getInterpFrac(double t, int& ind1, double& frac1, int& ind2, double& frac2, bool reverse) {
    if (keyframes.size() == 0) {
        return;
    }
    
    if (!reverse) {
        if (t >= keyframes[keyframes.size()-1].time) {
            ind1 = ind2 = keyframes.size()-1;
            frac1 = frac2 = 0.5;
            return;
        }
        if (t <= keyframes[0].time) {
            ind1 = ind2 = 0 ;
            frac1 = frac2 = 0.5;
            return;
        }
    } else {
        if (t > keyframes[keyframes.size()-1].time) {
            int round = (int)((t-keyframes[0].time)/(keyframes[keyframes.size()-1].time-keyframes[0].time));
            double resid = (t-keyframes[0].time) - round * (keyframes[keyframes.size()-1].time-keyframes[0].time);
            if (round % 2 == 0) {
                t = keyframes[0].time + resid;
            } else {
                t = keyframes[keyframes.size()-1].time - resid;
            }
        } else if (t < keyframes[0].time) {
            int round = (int)((keyframes[0].time-t)/(keyframes[keyframes.size()-1].time-keyframes[0].time));
            double resid = (keyframes[0].time-t) - round * (keyframes[keyframes.size()-1].time-keyframes[0].time);
            if (round % 2 == 1) {
                t = keyframes[0].time + resid;
            } else {
                t = keyframes[keyframes.size()-1].time - resid;
            }
        }
    }

    int min_ind = 0;
    int max_ind = keyframes.size()-1;
    while (max_ind - min_ind > 1) {
        if (t > keyframes[0.5*(max_ind + min_ind)].time) {
            min_ind = 0.5*(max_ind + min_ind);
        } else if (t == keyframes[0.5*(max_ind + min_ind)].time) {
            min_ind = 0.5*(max_ind + min_ind);
            max_ind = 0.5*(max_ind + min_ind);
        } else {
            max_ind = 0.5*(max_ind + min_ind);
        }
    }
    ind1 = min_ind;
    ind2 = max_ind;
 
    if (fabs((keyframes[ind2].time - keyframes[ind1].time)) < 0.000001) {
        cout << "close" << endl;
        frac1 = frac2 = 0.5;
    } else {
        frac2 = (t - keyframes[ind1].time) / (keyframes[ind2].time - keyframes[ind1].time);
        frac1 = 1 - frac2;
    }
}


