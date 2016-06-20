//
//  RigPart.cpp
//  physx_test
//
//  Created by YuWenhao on 10/9/15.
//  Copyright (c) 2015 YuWenhao. All rights reserved.
//

#include "RigPart.h"
#include <iostream>

using namespace std;
using namespace physx;

RigPart::~RigPart() {
    for (int i = 0; i < descriptors.size(); i++) {
        delete descriptors[i];
    }
}

void RigPart::updateContactForce(PxVec3 orig, PxVec3 force) {
    bool isinside = false;
    for (int i = 0; i < components.size(); i++) {
        PxBounds3 bound = components[i]->getWorldBounds();
        if (bound.contains(orig)) {
            isinside = true;
        }
    }
    if (isinside) {
        total_force -= force;
        total_torque -= (orig - ft_center).cross(force);
    }
}

void RigPart::addToScene(physx::PxScene* scene) {
    for (int i = 0; i < components.size(); i++) {
        scene->addActor(*components[i]);
    }
}


void RigPart::clearForceTorque() {
    total_force = PxVec3(0);
    total_torque = PxVec3(0);
}

void RigPart::translate(int ind, PxVec3 vec, bool mod_ft) {
    PxTransform current_pose = components[ind]->getGlobalPose();
    current_pose.p += vec;
    components[ind]->setGlobalPose(current_pose);
    
    descriptors[ind]->translation = current_pose.p;
    
    if (mod_ft) {
        ft_center += vec;
        
        for (int i = 0; i < gripper_constraints.size(); i++) {
            PxVec3 pos = gripper_constraints[i].second;
            gripper_previous[i] = pos;
            gripper_constraints[i].second = pos + vec;
        }
    }
}

void RigPart::rotate(int ind, PxReal angle, PxVec3 axis, bool mod_ft) {
    PxShape *shape_buffer[1];
    components[ind]->getShapes(shape_buffer, 1);
    PxTransform current_pose = shape_buffer[0]->getLocalPose();;
    current_pose.q = PxQuat(angle, axis) * current_pose.q;
    shape_buffer[0]->setLocalPose(current_pose);
    
    descriptors[ind]->rotation = current_pose.q;
    
    if (mod_ft) {
        ft_center = PxQuat(angle, axis).rotate(ft_center);
    }
}

void RigPart::rotateAround(int ind, PxReal angle, PxVec3 axis, PxVec3 center, bool mod_ft) {
    PxShape *shape_buffer[1];
    components[ind]->getShapes(shape_buffer, 1);
    PxTransform current_pose = shape_buffer[0]->getLocalPose();;
    current_pose.q = PxQuat(angle, axis) * current_pose.q;
    
    descriptors[ind]->rotation = current_pose.q;
    PxVec3 cur_cent = current_pose.p - center;
    cout << ind << " " << current_pose.p[0] << " " << current_pose.p[1] << " " << current_pose.p[2] << endl;
    PxVec3 new_cent = PxQuat(angle, axis).rotate(cur_cent);
    current_pose.p = current_pose.p + new_cent - cur_cent;

    descriptors[ind]->translation += new_cent - cur_cent;
    
    shape_buffer[0]->setLocalPose(current_pose);
    
    if (mod_ft) {
        ft_center = PxQuat(angle, axis).rotate(ft_center);
    }
}

void RigPart::translateTo(int ind, physx::PxVec3 vec, bool mod_ft) {
    PxTransform current_pose = components[ind]->getGlobalPose();
    
    if (mod_ft) {
        ft_center += vec - current_pose.p;
        
        for (int i = 0; i < gripper_constraints.size(); i++) {
            PxVec3 pos = gripper_constraints[i].second;
            gripper_previous[i] = pos;
            gripper_constraints[i].second = pos + vec - current_pose.p;
        }
    }
    
    current_pose.p = vec;
    components[ind]->setGlobalPose(current_pose);
    
    descriptors[ind]->translation = current_pose.p;
}

void RigPart::rotateTo(int ind, physx::PxReal angle, physx::PxVec3 axis, bool mod_ft) {
    PxShape *shape_buffer[1];
    components[ind]->getShapes(shape_buffer, 1);
    PxTransform current_pose = shape_buffer[0]->getLocalPose();;
    
    if (mod_ft) {
        ft_center = current_pose.q.rotateInv(ft_center);
        
        ft_center = PxQuat(angle, axis).rotate(ft_center);
    }
    
    current_pose.q = PxQuat(angle, axis);
    shape_buffer[0]->setLocalPose(current_pose);
    
    descriptors[ind]->rotation = current_pose.q;
}

void RigPart::recordInitTransform() {
    global_init_transforms.resize(components.size());
    local_init_transforms.resize(components.size());
    for (int i = 0; i < components.size(); i++) {
        global_init_transforms[i] = components[i]->getGlobalPose();
        PxShape *shape_buffer[1];
        components[i]->getShapes(shape_buffer, 1);
        local_init_transforms[i] = shape_buffer[0]->getLocalPose();
    }
    
    init_ft_center = ft_center;
}

void RigPart::reset() {
    for (int i = 0; i < components.size(); i++) {
        components[i]->setGlobalPose(PxTransform(PxVec3(0)));
        PxShape *shape_buffer[1];
        components[i]->getShapes(shape_buffer, 1);
        shape_buffer[0]->setLocalPose(PxTransform(local_init_transforms[i].q));
        descriptors[i]->translation = PxVec3(0);
        descriptors[i]->rotation = PxQuat(0, PxVec3(0, 0, 1));

        rotate(i, 0, PxVec3(0, 0, 1));
        translate(i, global_init_transforms[i].p);
    }
    
    ft_center = init_ft_center;
    
    recorded_torques.clear();
    recorded_forces.clear();
}

void RigPart::recordForceTorque() {
    recorded_forces.push_back(total_force);
    recorded_torques.push_back(total_torque);
}





