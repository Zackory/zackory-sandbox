//
//  Slider.h
//  physx_test
//
//  Created by YuWenhao on 1/20/16.
//  Copyright (c) 2016 YuWenhao. All rights reserved.
//

#ifndef __physx_test__Slider__
#define __physx_test__Slider__

#include <stdio.h>
#include <string.h>
#include "UiElement.h"

class Slider : public UiElement {
public:
    Slider() : UiElement("default") {}
    
    Slider(double startx, double starty, double w, double h, double max, double min, double init, std::string name);
    
    void initialize(double startx, double starty, double w, double h, double max, double min, double init, std::string name);
    
    void setRelatedValue(float* related_value) {
        _related_value = related_value;
    }
    
    bool IsClicked(double x, double y);
    
    void Render();
    
    double getValue();
private:
    double _startx;
    double _starty;
    double _width;
    double _height;
    
    double _value;
    double _max_value;
    double _min_value;
    
    float* _related_value;
};

#endif /* defined(__physx_test__Slider__) */
