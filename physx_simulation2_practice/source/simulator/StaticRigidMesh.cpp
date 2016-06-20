//
//  StaticRigidMesh.cpp
//  physx_test
//
//  Created by YuWenhao on 10/1/15.
//  Copyright (c) 2015 YuWenhao. All rights reserved.
//

#include "StaticRigidMesh.h"
#include "MeshReader.h"
#include "../renderer/Renderer.h"
#include <PxToolkit.h>
#include <iostream>

using namespace std;
using namespace physx;

StaticRigidMesh::~StaticRigidMesh() {
    mRigid->release();
}

void StaticRigidMesh::createStaticRigidMesh(PxPhysics* physx, PxCooking* cooking, PxTransform transform, PxMeshScale scale) {
    mRigid = physx->createRigidStatic(transform);
    
    // create regular mesh
    PxU32 numTriangles = mesh.faces.size();
    int numParticles = mesh.particles.size();
    
    // create cloth particles
    PxVec3* particles = new PxVec3[numParticles];
    
    // create quads
    PxU32* triangles = new PxU32[3*numTriangles];
    
    PxVec3* pIt = particles;
    for(PxU32 i=0; i<numParticles; ++i, ++pIt)
    {
        *pIt = PxVec3(mesh.particles[i].pos(0), mesh.particles[i].pos(1), mesh.particles[i].pos(2));
    }
    
    PxU32* iIt = triangles;
    for(PxU32 i=0; i<mesh.faces.size(); ++i)
    {
        *iIt++ = mesh.faces[i].particles[0]->index;
        *iIt++ = mesh.faces[i].particles[1]->index;
        *iIt++ = mesh.faces[i].particles[2]->index;
    }
    
    // create fabric from mesh
    PxTriangleMeshDesc meshDesc;
    meshDesc.points.count = numParticles;
    meshDesc.points.stride = sizeof(PxVec3);
    meshDesc.points.data = particles;
    
    meshDesc.triangles.count = numTriangles;
    meshDesc.triangles.stride = 3*sizeof(PxU32);
    meshDesc.triangles.data = triangles;
    
    PxToolkit::PxDefaultMemoryOutputStream writeBuffer;
    bool status = cooking->cookTriangleMesh(meshDesc, writeBuffer);
    if(!status) {
        cout << "cook triangle mesh failed\n";
        return;
    }
    
    PxToolkit::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
    PxTriangleMesh* triangle_mesh = physx->createTriangleMesh(readBuffer);
    
    PxMaterial* material = physx->createMaterial(1.0f,1.0f,0.5f);
    
    mRigid->createShape(PxTriangleMeshGeometry(triangle_mesh, scale), *material);
    
    delete[] particles;
    delete[] triangles;
}

void StaticRigidMesh::loadMesh(std::string filename) {
    MeshReader reader;
    reader.readObjMesh(mesh, filename);
}



