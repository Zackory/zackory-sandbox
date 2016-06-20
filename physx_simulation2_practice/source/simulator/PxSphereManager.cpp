//
//  PxSphereManager.cpp
//  physx_test
//
//  Created by YuWenhao on 2/24/16.
//  Copyright (c) 2016 YuWenhao. All rights reserved.
//

#include "PxSphereManager.h"

using namespace physx;
using namespace std;

PxSphereManager* PxSphereManager::mSphereManager = NULL;

PxSphereManager* PxSphereManager::GetSingleton() {
    if (!mSphereManager)
    {
        mSphereManager = new PxSphereManager();
    }
    return mSphereManager;
}

void PxSphereManager::DestroySingleton() {
    if (mSphereManager) {
        delete mSphereManager;
    }
}

void PxSphereManager::addSphere(physx::PxVec3 center, double radius) {
    if (mNumSphere < 32) {
        spheres[mNumSphere++] = PxClothCollisionSphere(center, radius);
    }
}

void PxSphereManager::addCapsule(uint32_t first, uint32_t second) {
    capsules.push_back(make_pair(first, second));
}

void PxSphereManager::setUpCapsules(PxCloth* cloth) {
    cloth->setCollisionSpheres(spheres, mNumSphere);
    
    for (int i = 0; i < capsules.size(); i++) {
        cloth->addCollisionCapsule(capsules[i].first, capsules[i].second);
    }
}

void PxSphereManager::updateCapsules(PxCloth* cloth) {
    cloth->setCollisionSpheres(spheres, mNumSphere);
}

void PxSphereManager::getSphereData(vector<PxClothCollisionSphere>& sp, vector<pair<uint32_t, uint32_t> >& cp) {
    sp.clear();
    cp.clear();
    
    for (int i = 0; i < mNumSphere; i++) {
        sp.push_back(PxClothCollisionSphere(spheres[i].pos, spheres[i].radius));
    }
    for (int i = 0; i < capsules.size(); i++) {
        cp.push_back(make_pair(capsules[i].first, capsules[i].second));
    }
}


void PxSphereManager::translate(int ind, PxVec3 vec) {
    spheres[ind].pos += vec;
}

void PxSphereManager::rotate(int ind, PxReal angle, PxVec3 axis) {
    spheres[ind].pos = PxQuat(angle, axis).rotate(spheres[ind].pos);
}

void PxSphereManager::rotateAround(int ind, PxReal angle, PxVec3 axis, PxVec3 center) {
    spheres[ind].pos = PxQuat(angle, axis).rotate(spheres[ind].pos - center) + center;
}

void PxSphereManager::recordInitSpheres() {
    for (int i = 0 ; i < mNumSphere; i++) {
        initspheres[i].pos = spheres[i].pos;
        initspheres[i].radius = spheres[i].radius;
    }
}

void PxSphereManager::resetPos() {
    for (int i = 0 ; i < mNumSphere; i++) {
        spheres[i].pos = initspheres[i].pos;
        spheres[i].radius = initspheres[i].radius;
    }
}

void PxSphereManager::clearSpheres() {
    mNumSphere = 0;
}

