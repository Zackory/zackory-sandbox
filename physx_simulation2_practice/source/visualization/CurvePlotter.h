//
//  CurvePlotter.h
//  physx_test
//
//  Created by YuWenhao on 2/16/16.
//  Copyright (c) 2016 YuWenhao. All rights reserved.
//

#ifndef __physx_test__CurvePlotter__
#define __physx_test__CurvePlotter__

#include <stdio.h>
#include <vector>
#include <Eigen/Dense>
#include <string>

class CurvePlotter {
public:
    CurvePlotter();
    
    void draw();
    
    void updateData(Eigen::Vector2d);

    void setColor(Eigen::Vector3d);
    
    void setLineWidth(int);
    
    void setScale(Eigen::Vector2d, Eigen::Vector2d);
    
    void setScreenPlace(Eigen::Vector2d, Eigen::Vector2d);
    
    void setTitle(std::string);
    
    void toggleDyanmicChange();
    
    void clearData();
    
    Eigen::Vector2d getXscale() {return mXscale;}
    Eigen::Vector2d getYscale() {return mYscale;}
protected:
    Eigen::Vector2d mBegin;
    Eigen::Vector2d mEnd;
    
    Eigen::Vector3d mCurveColor;
    int mCurveWidth;
    Eigen::Vector2d mXscale;
    Eigen::Vector2d mYscale;
    bool isdynamic;
    
    std::vector<Eigen::Vector2d> mData;
    
    std::string mTitle;
};

#endif /* defined(__physx_test__CurvePlotter__) */
