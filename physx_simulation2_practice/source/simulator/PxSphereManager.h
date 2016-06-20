//
//  PxSphereManager.h
//  physx_test
//
//  Created by YuWenhao on 2/24/16.
//  Copyright (c) 2016 YuWenhao. All rights reserved.
//

#ifndef __physx_test__PxSphereManager__
#define __physx_test__PxSphereManager__

#include <stdio.h>
#include <vector>
#include <PxPhysicsAPI.h>

class PxSphereManager {
public:
    PxSphereManager() {
        mSphereManager = NULL;
    }
    
    static PxSphereManager* GetSingleton();
    static void DestroySingleton();
    
    void addSphere(physx::PxVec3 center, double radius);
    
    void addCapsule(uint32_t first, uint32_t second);
    
    void setUpCapsules(physx::PxCloth*);
    
    void updateCapsules(physx::PxCloth*);
        
    void getSphereData(std::vector<physx::PxClothCollisionSphere>&, std::vector<std::pair<uint32_t, uint32_t> >&);
    
    void translate(int, physx::PxVec3);
    
    void rotate(int, physx::PxReal, physx::PxVec3);
    
    void rotateAround(int, physx::PxReal, physx::PxVec3, physx::PxVec3);
    
    int getNumSphere() {
        return mNumSphere;
    }
    
    void recordInitSpheres();
    
    void resetPos();
    
    void clearSpheres();
private:
    static PxSphereManager* mSphereManager;
    
    physx::PxClothCollisionSphere spheres[32];
    int mNumSphere = 0;
    physx::PxClothCollisionSphere initspheres[32];
    
    std::vector<std::pair<uint32_t, uint32_t> > capsules;
};

#endif /* defined(__physx_test__PxSphereManager__) */
