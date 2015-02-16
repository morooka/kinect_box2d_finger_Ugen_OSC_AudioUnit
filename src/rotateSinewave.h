//
//  rotateSinewave.h
//  emptyExample
//
//  Created by mitsuo morooka on 2014/04/25.
//
//

#pragma once

#include "ofxUGen.h"

using namespace ofxUGen;

class rotateSinewave : public ofxUGen::SynthDef
{
public:
    
	ofVec3f color1,color2,color3;
    ofVec2f circlePosition,pos;
    
	UGen envgen;
	UGen amp;
    
    float count;
    
    ofVec2f enMove1,enMove2,enMove3;
    
    int rotation,radius;
    
    rotateSinewave(int x,int y,float z,int freq1,int freq2)
    {
        pos.x = x;
        pos.y = y;
        
        float freq = ofMap(pos.y,0, ofGetHeight(),freq1,freq2);   //freq1 = 6000,freq2 = 12000
        float pan = ofMap(pos.x, 0,ofGetWidth(), -1, 1);
       // float volume = ofMap(pos.z,50, 700, 300, 1);
        
        Env env = Env::perc(0.1,0.2,0.1, EnvCurve::Numerical);  //Sine
        envgen = EnvGen::AR(env);
        envgen.addDoneActionReceiver(this);
        
        amp =SinOsc::AR(ofRandom(4.0), 0, 0.4, 0.4) *  envgen;
        Out(
            Pan2::AR(SinOsc::AR(freq) * amp * z , pan)    //volume2=100
            );
    }
    
    void draw(int cF1X,int cF1Y)
	{
        count = count + 0.2f*amp.getValue()*10;
        
        enMove1.x = std::cos(count)*rotation*amp.getValue()*2;    //rotation =2000
        enMove1.y = std::sin(count)*rotation*amp.getValue()*2;
        
        enMove2.x = std::cos(count+400)*rotation*amp.getValue()*2;
        enMove2.y = std::sin(count+400)*rotation*amp.getValue()*2;
        
        enMove3.x = std::cos(count+800)*rotation*amp.getValue()*2;
        enMove3.y = std::sin(count+800)*rotation*amp.getValue()*2;
        
        
        
		ofFill();
		ofSetColor(color1.x,color1.y,color1.z+30);                             //253,217,217
        ofCircle(cF1X+enMove1.x,cF1Y+enMove1.y,radius*amp.getValue()*2);   //radius = 200;
        ofSetColor(color1.x,color1.y,color1.z+30);                             //220,214,217
        ofCircle(cF1X+enMove2.x,cF1Y+enMove2.y,radius*amp.getValue()*2);
        ofSetColor(color1.x,color1.y,color1.z+30);                             //253,217,217
        ofCircle(cF1X+enMove3.x,cF1Y+enMove3.y,radius*amp.getValue()*2);
	}
    
    bool isAlive()
	{
		return !Out().isNull();
	}
};
