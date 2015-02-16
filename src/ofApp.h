#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxBox2d.h"
#include "ofxUGen.h"
#include "ofxKinect.h"
#include "ofxSyphon.h"
#include "ofxFingerDetector.h"
#include "ofxOsc.h"
#include "myLFPulse.h"
#include "rotateSinewave.h"
#include "myLowSynth.h"
#include "CustomCircle.h"

#include "ofxAudioUnit.h"

#define PORT 12345

class SoundData {
public:
    int	 soundID;
    bool bHit;
    bool bBoxHit;
};

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    void exit();
    void rotateSinewaveDraw();
    void lowSynthsDraw();
    void contourLine();
    void myLFPulseDraw();
    
    void contactStart(ofxBox2dContactArgs &e);
    void contactEnd(ofxBox2dContactArgs &e);
    
    void audioReceived(float * input, int bufferSize, int nChannels);
    
    ofxKinect kinect;
    
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage; // grayscale depth image
    ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
    ofxCvGrayscaleImage grayThreshFar; // the far thresholded image
    ofxCvContourFinder contourFinder;
    
    int nearThreshold;
    int farThreshold;
    
    int angle;
    
    ofxSyphonServer mainOutputSyphonServer;
    
    deque <rotateSinewave*> rotateSinewavesD;
    
    bool   bSound = false;
    
    ofxBox2d     box2d;			  //	the box2d world
    ofPolyline   drawing, drawing2, drawing3, drawing4, drawing5;		  //	we draw with this first
    ofxBox2dEdge edgeLine, edgeLine2 , edgeLine3, edgeLine4, edgeLine5;
    
    vector		<ofPtr<ofxBox2dCircle> >	circles;
    vector      <ofPtr<ofxBox2dRect> >      boxes;
    
    vector <CustomCircle *> CustomCircles;
    
    //ofVec2f p,p2;
    
    ofxFingerDetector         fingerFinder,fingerFinder2;
    
    int numFingerPoints = 0,numFingerPoints2 = 0;
    
    float adjustX = 0;
    float adjustY = 0;
    
    int fingerNUM1 = 0, fingerNUM2 = 0; //それぞれの指の数
    
    bool bContourLine = false;
    
    deque<ofVec2f> pathVerticesT;
    ofMesh pathLinesT;
    
    deque <myLFPulse*>      myLFPulses;
    
    bool bText = true;
    bool bFingerLine = false;
    
    bool fingersFound=false;
    bool fingersFound2=false;
    
    bool bAddBaloon = false;
    
    ofVec2f hand1finger1 = ofVec2f(0,0);
    ofVec2f hand1finger2 = ofVec2f(0,0);
    ofVec2f hand1finger3 = ofVec2f(0,0);
    ofVec2f hand1finger4 = ofVec2f(0,0);
    ofVec2f hand1finger5 = ofVec2f(0,0);
    
    ofVec2f hand2finger1 = ofVec2f(0,0);
    ofVec2f hand2finger2 = ofVec2f(0,0);
    ofVec2f hand2finger3 = ofVec2f(0,0);
    ofVec2f hand2finger4 = ofVec2f(0,0);
    ofVec2f hand2finger5 = ofVec2f(0,0);
    
    float fingerBaloonDist[30];
    
    float counter;
    
    ofxOscReceiver receiver;
    
    int bufSize;
    float * left;
    float * right;
    
    ofxAudioUnit compressor;
    ofxAudioUnit delay,delay2;
    ofxAudioUnit distortion;
    ofxAudioUnit filter;
    ofxAudioUnit reverve, reverve2;
    
    //ofxAudioUnitFilePlayer source1, source2, source3;
    
    ofxAudioUnitFilePlayer source[6];
    ofxAudioUnitMixer mixer;
    ofxAudioUnitOutput output;
    
    ofxAudioUnitTap tap1, tap2, tap3, tap4, tap5;
    ofPolyline wave1, wave2, wave3, wave4, wave5;
    
    bool bAudioUniteOn[30];
    bool bAudioUniteOff = true;
    
    bool bAllBaloonSound = false;
    
    ofVec2f pointer = ofVec2f(0,0);
};
