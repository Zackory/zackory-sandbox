//
//  CurvePlotter.cpp
//  physx_test
//
//  Created by YuWenhao on 2/16/16.
//  Copyright (c) 2016 YuWenhao. All rights reserved.
//

#include "CurvePlotter.h"

// Include OpenGL based on OS
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <GLUT/glut.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/glut.h>
#endif

#include <iostream>

using namespace std;
using namespace Eigen;

//Function for rendering string on screen
static void RenderBitmapString(float x, float y, void *font,char *string)
{
    char *c;
    glRasterPos2f(x, y);
    for (c=string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

CurvePlotter::CurvePlotter() {
    mCurveWidth = 1;
    mCurveColor = Vector3d(0.5, 0.5, 0.5);
    isdynamic = false;
}

void CurvePlotter::draw() {
    // draw background
    glColor3d(1, 1, 1);
    glBegin(GL_QUADS);
    glVertex2d(mBegin(0), mBegin(1));
    glVertex2d(mBegin(0), mEnd(1));
    glVertex2d(mEnd(0), mEnd(1));
    glVertex2d(mEnd(0), mBegin(1));
    glEnd();
    
    // draw axes
    glColor3d(0, 0, 0);
    glBegin(GL_LINES);
    glVertex2d(mBegin(0), mBegin(1));
    glVertex2d(mBegin(0), mEnd(1));
    glVertex2d(mBegin(0), mEnd(1) - (- mYscale(0)) / (mYscale(1) - mYscale(0)) * (mEnd(1) - mBegin(1)));
    glVertex2d(mEnd(0), mEnd(1) - (- mYscale(0)) / (mYscale(1) - mYscale(0)) * (mEnd(1) - mBegin(1)));
    glEnd();
    
    // draw data
    glColor3d(mCurveColor(0), mCurveColor(1), mCurveColor(2));
    glLineWidth(mCurveWidth);
    
    for (int i = int(mData.size())-2; i >= 0; i--) {
        double posx1 = (mData[i](0) - mXscale(0) - mData[0](0)) / (mXscale(1) - mXscale(0)) * (mEnd(0) - mBegin(0)) + mBegin(0);
        double posy1 = mEnd(1) - (mData[i](1) - mYscale(0)) / (mYscale(1) - mYscale(0)) * (mEnd(1) - mBegin(1));
        
        double posx2 = (mData[i+1](0) - mXscale(0) - mData[0](0)) / (mXscale(1) - mXscale(0)) * (mEnd(0) - mBegin(0)) + mBegin(0);
        double posy2 = mEnd(1) - (mData[i+1](1) - mYscale(0)) / (mYscale(1) - mYscale(0)) * (mEnd(1) - mBegin(1));
        
        if (posx1 < mBegin(0)) {
            break;
        }
        glBegin(GL_LINES);
        glVertex2d(posx1, posy1);
        glVertex2d(posx2, posy2);
        glEnd();
    }
    glLineWidth(1);
    
    glColor3d(0.1, 0.1, 0.1);
    int* pFont=(int*)GLUT_BITMAP_8_BY_13;
    char s_tmp[256];
    strcpy(s_tmp, mTitle.c_str());
    RenderBitmapString(0.5*(mBegin(0) + mEnd(0)), mBegin(1) + (mEnd(1)-mBegin(1))*0.2, pFont, s_tmp);
}

void CurvePlotter::updateData(Vector2d d) {
    mData.push_back(d);
    if (mData.size() > 1 && isdynamic) {
        mXscale(0) += d(0) - mData[mData.size()-2](0);
        mXscale(1) += d(0) - mData[mData.size()-2](0);
    }
}

void CurvePlotter::setColor(Vector3d color) {
    mCurveColor = color;
}

void CurvePlotter::setLineWidth(int w) {
    mCurveWidth = w;
}

void CurvePlotter::setScale(Vector2d xscale, Vector2d yscale) {
    mXscale = xscale;
    mYscale = yscale;
}

void CurvePlotter::setScreenPlace(Vector2d begin, Vector2d end) {
    mBegin = begin;
    mEnd = end;
}

void CurvePlotter::toggleDyanmicChange() {
    isdynamic = !isdynamic;
}

void CurvePlotter::clearData() {
    mData.clear();
}

void CurvePlotter::setTitle(string title) {
    mTitle = title;
}



