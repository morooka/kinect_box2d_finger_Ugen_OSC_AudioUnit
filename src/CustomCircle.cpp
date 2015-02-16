//
//  CustomCircle.cpp
//  box2d_UGen_Leap
//
//  Created by 諸岡 光男 on 2013/11/02.
//
//

#include "CustomCircle.h"

CustomCircle::CustomCircle(int _num){
    num = _num;
}

void CustomCircle::draw(){
    float count;
    count += 0.03f;
    
    
    ofNoFill();
    ofSetColor(255,204,0);
    ofCircle(getPosition().x, getPosition().y,std::sin(count)*250);
   // ofCircle(getPosition().x, getPosition().y,18);

}
