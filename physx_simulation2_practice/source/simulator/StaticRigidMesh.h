//
//  StaticRigidMesh.h
//  physx_test
//
//  Created by YuWenhao on 10/1/15.
//  Copyright (c) 2015 YuWenhao. All rights reserved.
//

#ifndef __physx_test__StaticRigidMesh__
#define __physx_test__StaticRigidMesh__

#include <stdio.h>
#include "Mesh.h"
#include <PxPhysicsAPI.h>

class StaticRigidMesh {
public:
    ~StaticRigidMesh();
    
    void createStaticRigidMesh(physx::PxPhysics*, physx::PxCooking*, physx::PxTransform transform = physx::PxTransform(), physx::PxMeshScale scale = physx::PxMeshScale());
    
    Mesh mesh;
    
    void loadMesh(std::string filename);
    
    physx::PxRigidStatic* mRigid;
};

#endif /* defined(__physx_test__StaticRigidMesh__) */
