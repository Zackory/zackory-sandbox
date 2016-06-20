//
//  Renderer.h
//  physx_test
//
//  Created by YuWenhao on 10/1/15.
//  Copyright (c) 2015 YuWenhao. All rights reserved.
//

#ifndef __physx_test__Renderer__
#define __physx_test__Renderer__

#include <stdio.h>
#include "../utils/opengl.h"
#include "../simulator/Mesh.h"

#include "../simulator/RigPart.h"

class Renderer {
public:
    void renderMesh(Mesh&, double color[3], bool wire = true, bool seqinv = false);
    
    void renderRigPart(RigPart*, double color[3]);
    
    void renderShape(RenderDescriptor*, double color[3]);
    
    void renderSphereManager(double color[3]);
};


#endif /* defined(__physx_test__Renderer__) */
