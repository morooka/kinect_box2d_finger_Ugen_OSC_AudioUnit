//
//  CustomRect.cpp
//  box2d_UGen_Leap
//
//  Created by 諸岡 光男 on 2013/11/02.
//
//

#include "CustomRect.h"

CustomRect::CustomRect(int _num){
    num = _num;
}
void CustomRect::draw(){
    
    ofNoFill();
    ofSetColor(102,204,153);
    ofRect(getPosition().x, getPosition().y, 15, 15);
    
}