//
//  MyLowSynth.h
//  kinect_box2d_finger_Ugen
//
//  Created by 諸岡 光男 on 2015/02/11.
//
//
class myLowSynth : public ofxUGen::SynthDef
{
public:
    
    ofVec2f pos;
    
    UGen envgen;
    UGen amp;
    
    myLowSynth(int x,int y)
    {
        pos.x = x;
        pos.y = y;
        
        float freq = ofMap(pos.y, 0, ofGetHeight(), 50, 400);
        float pan = ofMap(pos.x, 0, ofGetWidth(), -1, 1);
        
        Env env = Env::perc(0.5, 1.5, 0.3, EnvCurve::Sine);
        envgen = EnvGen::AR(env);
        envgen.addDoneActionReceiver(this);
        
        amp = SinOsc::AR(ofRandom(4.0), 0, 0.5, 0.5) * envgen;
        
        Out(
            Pan2::AR(SinOsc::AR(freq) * amp * 1.1f, pan)
            );
    }
    
    void draw()
    {
        ofNoFill();
        ofSetColor(150,200,200,100);
        ofCircle(pos.x, pos.y, amp.getValue() * 150);
    }
};
