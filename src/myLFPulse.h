//
//  myLFPulse.h
//  kinect_box2d_finger_Ugen
//
//  Created by 諸岡 光男 on 9/23/14.
//
//

#pragma onse

#include "ofxUGen.h"

class myLFPulse : public ofxUGen::SynthDef
{
public:
	
	ofVec3f pos;     
	
	UGen envgen;
	UGen amp;
    
    int count;
    
	myLFPulse(int x, int y,float z)
	{
		pos.x = x;
		pos.y = y;
        pos.z = z;
		
		float freq = ofMap(pos.y, 0, ofGetHeight(), 50, 5000);
		float pan = ofMap(pos.x, 0, ofGetWidth(), -1, 1);
        float volume = ofMap(pos.z,0.1f,1.0f,0.1f, 1.0f);
        
		Env env = Env::perc(0.6, 1.0, 0.5, EnvCurve::Welch);  //sine // Numerical :Welch
		envgen = EnvGen::AR(env);
		envgen.addDoneActionReceiver(this);
		
		amp = LFPulse::AR() * envgen;   //SinOsc::AR(ofRandom(4,0), 0, 0.5, 0.5) FSinOsc SinOsc LFPulse
        // amp = SinOsc::AR(0, 0, 0.5, 0.5);
		
        Out(
            Pan2::AR(SinOsc::AR(freq)*amp*volume*0.2f, pan)
            );
    }
	void draw(int x,int y,int z)
	{
		/*ofFill();
        glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
		ofSetColor(50,233,50,200*amp.getValue());
		ofDrawSphere(x,y,z, amp.getValue() * 50);
         */
	}
    
    bool isAlive()
	{
		return !Out().isNull();
	}
};
