//
//  Slider.cpp
//  physx_test
//
//  Created by YuWenhao on 1/20/16.
//  Copyright (c) 2016 YuWenhao. All rights reserved.
//

#include "Slider.h"

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

using namespace std;

//Function for rendering string on screen
static void RenderBitmapString(float x, float y, void *font,char *string)
{
    char *c;
    glRasterPos2f(x, y);
    for (c=string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

Slider::Slider(double startx, double starty, double w, double h, double max, double min, double init, string name) : UiElement(name) {
    _startx = startx;
    _starty = starty;
    _width = w;
    _height = h;
    _max_value = max;
    _min_value = min;
    _value = init;
}

void Slider::initialize(double startx, double starty, double w, double h, double max, double min, double init, std::string name) {
    _name = name;
    _startx = startx;
    _starty = starty;
    _width = w;
    _height = h;
    _max_value = max;
    _min_value = min;
    _value = init;
}

bool Slider::IsClicked(double x, double y) {
    if (x >= _startx && x < _startx + _width
        && y >= _starty && y < _starty + _height) {
        _value = (x - _startx) * 1.0 / _width * (_max_value - _min_value) + _min_value;
        
        *_related_value = _value;
        return true;
    }
    return false;
}

void Slider::Render() {
    glColor3d(0.6, 0.9, 0.6);
    glBegin(GL_QUADS);
    glVertex2d(_startx, _starty);
    glVertex2d(_startx + (_value-_min_value)*1.0/(_max_value-_min_value)*_width, _starty);
    glVertex2d(_startx + (_value-_min_value)*1.0/(_max_value-_min_value)*_width, _starty+_height);
    glVertex2d(_startx, _starty + _height);
    glEnd();
    
    glColor3d(0.1, 0.1, 0.1);
    int* pFont=(int*)GLUT_BITMAP_8_BY_13;
    char s_tmp[256];
    strcpy(s_tmp, (_name + ": " + to_string(_value)).c_str());
    RenderBitmapString(_startx + (_value-_min_value)*1.0/(_max_value-_min_value)*_width + 0.005, _starty + _height*2/3, pFont, s_tmp);
    
}

double Slider::getValue() {
    return _value;
}
