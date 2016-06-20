//
//  UiElement.h
//  physx_test
//
//  Created by YuWenhao on 1/20/16.
//  Copyright (c) 2016 YuWenhao. All rights reserved.
//

#ifndef physx_test_UiElement_h
#define physx_test_UiElement_h

#include <string>

class UiElement {
public:
    UiElement(std::string name) : _name(name) {}
    
    virtual bool IsClicked(double x, double y) = 0;
    
    virtual void Render() = 0;
    
protected:
    std::string _name;
};

#endif
