//
//  CustomRect.h
//  box2d_UGen_Leap
//
//  Created by 諸岡 光男 on 2013/11/02.
//
//
#pragma once

#include "ofxBox2d.h"

class CustomRect : public ofxBox2dRect {
    
public:
    CustomRect(int num);
    
    void draw();
    
    int num;
    
};
