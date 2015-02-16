#include "ofApp.h"

using namespace ofxUGen;

class MySynth : public ofxUGen::SynthDef
{
public:
    
    ofVec2f pos;
    
    UGen envgen;
    UGen amp;
    
    MySynth(int x,int y)
    {
        pos.x = x;
        pos.y = y;
        
        float freq = ofMap(pos.x, 0, ofGetHeight(), 0, 2000);
        float pan = ofMap(pos.y, 0, ofGetWidth(), -1, 1);
        
        Env env = Env::perc(0.5, 1.0f, 0.3, EnvCurve::Sine);
        envgen = EnvGen::AR(env);
        envgen.addDoneActionReceiver(this);
        
        amp = SinOsc::AR(ofRandom(4.0), 0, 0.5, 0.5) * envgen;
        
        Out(
            Pan2::AR(SinOsc::AR(freq) * amp * 0.8, pan)
            );
    }
    
    void draw()
    {
        
        ofNoFill();
        ofSetColor(200,255,200,100);
        ofCircle(pos.x, pos.y, amp.getValue() * 150);
    }
};

vector<MySynth*> synths;
deque<myLowSynth*> lowSynths;

static bool removeShapeOffScreen(ofPtr<ofxBox2dBaseShape> shape) {
    if (!ofRectangle(0, -400, ofGetWidth(), ofGetHeight()+400).inside(shape.get()->getPosition())) {
        return true;
    }
    return false;
}


//--------------------------------------------------------------
void ofApp::setup(){
   
    source[1].setFile(ofFilePath::getAbsolutePath("water.wav"));
    source[2].setFile(ofFilePath::getAbsolutePath("piano.wav"));
    source[3].setFile(ofFilePath::getAbsolutePath("guitar.wav"));
    source[4].setFile(ofFilePath::getAbsolutePath("oil.wav"));
    source[5].setFile(ofFilePath::getAbsolutePath("ooki.wav"));

    
    distortion.setup(kAudioUnitType_Effect, kAudioUnitSubType_Distortion);
    delay.setup(kAudioUnitType_Effect, kAudioUnitSubType_Delay);
    delay2.setup(kAudioUnitType_Effect, kAudioUnitSubType_Delay);
    filter.setup(kAudioUnitType_Effect, kAudioUnitSubType_LowPassFilter);
    reverve.setup(kAudioUnitType_Effect, kAudioUnitSubType_MatrixReverb);
    reverve2.setup(kAudioUnitType_Effect, kAudioUnitSubType_MatrixReverb);

    
    source[1].connectTo(reverve).connectTo(tap1);
    source[2].connectTo(delay).connectTo(tap2);
    source[3].connectTo(filter).connectTo(tap3);
    source[4].connectTo(reverve2).connectTo(tap4);
    source[5].connectTo(delay2).connectTo(tap5);
    
    mixer.setInputBusCount(5);
    tap1.connectTo(mixer, 0);
    tap2.connectTo(mixer, 1);
    tap3.connectTo(mixer, 2);
    tap4.connectTo(mixer, 3);
    tap5.connectTo(mixer, 4);
    
    compressor.setup(kAudioUnitType_Effect, kAudioUnitSubType_DynamicsProcessor);
    mixer.connectTo(compressor).connectTo(output);
    
    mixer.setInputVolume(0.7, 2);
    
    output.start();
    
    source[1].stop();
    source[2].stop();
    source[3].stop();
    source[4].stop();
    source[5].stop();

    ofSetLogLevel(OF_LOG_VERBOSE);
    ofSetCircleResolution(60);
    ofSetVerticalSync(true);
   // ofSetFrameRate(120);
    
    kinect.setRegistration(true);
    
    kinect.init(false,false,false);
    
    kinect.open();
    
    colorImg.allocate(kinect.width, kinect.height);
    grayImage.allocate(kinect.width, kinect.height);
    grayThreshNear.allocate(kinect.width, kinect.height);
    grayThreshFar.allocate(kinect.width, kinect.height);
    
    nearThreshold = 230;
    farThreshold = 211;
    
    angle = 0;
    kinect.setCameraTiltAngle(angle);
    
    mainOutputSyphonServer.setName("Screen Output");
    
    box2d.init();
    box2d.enableEvents();   // <-- turn on the event listener
    box2d.setGravity(0, 10);
    //box2d.createBounds();
    //box2d.setFPS(120.0);
    box2d.registerGrabbing();
    
    // register the listener so that we get the events
    ofAddListener(box2d.contactStartEvents, this, &ofApp::contactStart);
    ofAddListener(box2d.contactEndEvents, this, &ofApp::contactEnd);
    
    edgeLine.setPhysics(0.0, 0.5, 0.5);
    edgeLine.create(box2d.getWorld());
    
    s().setup();
    
    ofHideCursor();
    
    pathLinesT.setMode(OF_PRIMITIVE_LINE_LOOP);
    
    ofBackground(0);
    
    adjustX = (float)ofGetWidth()/640.0f;
    adjustY = (float)ofGetHeight()/480.0f;
    
    ofSetLineWidth(1);
    
    receiver.setup(PORT);
    
    ofSoundStreamSetup(0, 2);
    bufSize = 512;
    
    left = new float[bufSize];
    right = new float[bufSize];
    
    for (int i = 0; i < 30; i++) {
        fingerBaloonDist[i] = 500;
        bAudioUniteOn[i] = false;
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    
    if(fingerNUM1 == 1){
        counter += 0.03f;
    }else{
        counter = 0;
    }
    
    ofRemove(circles, removeShapeOffScreen);
    ofRemove(boxes, removeShapeOffScreen);
    
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    
    while(receiver.hasWaitingMessages()){
        ofxOscMessage m;
        receiver.getNextMessage(&m);
        
        if(m.getAddress() == "/1/toggle1"){ //bSound==trueの場合grayImage描画,falseの場合grayImage描画無し＋発音
            bSound = m.getArgAsFloat(0);
        }
        if(m.getAddress() == "/1/toggle2"){ //風船追加モード
            bAddBaloon = m.getArgAsFloat(0);
        }
        
        if(m.getAddress() == "/1/toggle3"){ // OSCでaudioUniteテスト
            bAllBaloonSound = m.getArgAsFloat(0);
        }
        
        if(m.getAddress() == "/1/toggle4"){ // テキストの表示／非表示
            bText = m.getArgAsFloat(0);
        }
        
        if (m.getAddress() == "/3/xy") {
            pointer.x = (int)(ofGetWidth() * (1.0 - m.getArgAsFloat(0)));
            pointer.y = (int)(ofGetHeight() * (1.0 - m.getArgAsFloat(1)));
        }
        
        if(m.getAddress() == "/3/toggle1"){ // テキストの表示／非表示
            float r = ofRandom(2, 10);		// a random radius 4px - 20px
            ofPtr <ofxBox2dCircle> c = ofPtr <ofxBox2dCircle>(new ofxBox2dCircle);
            c.get()->setPhysics(1, 0.5, 0.9);
            c.get()->setup(box2d.getWorld(), pointer.x, pointer.y, ofRandom(2, 10));
            c.get()->setData(new SoundData());
            circles.push_back(c);
        }
        if(m.getAddress() == "/3/toggle2"){ // テキストの表示／非表示
            float r = ofRandom(2, 10);		// a random radius 4px - 20px
            ofPtr <ofxBox2dRect> c = ofPtr <ofxBox2dRect>(new ofxBox2dRect);
            c.get()->setPhysics(1, 0.5, 0.9);
            c.get()->setup(box2d.getWorld(), pointer.x, pointer.y, 6+ofRandom(-2,2), 6+ofRandom(-2,2));
            c.get()->setData(new SoundData());
            boxes.push_back(c);
        }
    }
    
    if(kinect.isFrameNew()) {
        grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width , kinect.height);
        unsigned char * pix = grayImage.getPixels();
        int numPixels = grayImage.getWidth() * grayImage.getHeight();
        for(int i = 0; i < numPixels; i++) {
            if(pix[i] < nearThreshold && pix[i] > farThreshold) {
                pix[i] = 255;
            } else {
                pix[i] = 0;
            }
        }
        
        grayImage.flagImageChanged();
        contourFinder.findContours(grayImage, 5, (kinect.width*kinect.height)/2, 7, false,false);
        
        if(contourFinder.nBlobs>0){
            edgeLine.destroy();
            ofPoint p;
            for(int i = 0 ; i < contourFinder.blobs[0].pts.size();i += 5){
                p.x=contourFinder.blobs[0].pts[i].x * adjustX;
                p.y=contourFinder.blobs[0].pts[i].y * adjustY;
                drawing.addVertex(p.x, p.y);
            }
            drawing.simplify();
            drawing.setClosed(true);
            
            edgeLine.addVertexes(drawing);
            edgeLine.simplify();
            edgeLine.setPhysics(0.0, 0.5, 0.5);
            edgeLine.create(box2d.getWorld());
            
            SoundData * data = (SoundData*)edgeLine.getData();
            drawing.clear();
            
            if (numFingerPoints > 0 ) {
                float strength = 16.0f;
                float damping  = 0.7f;
                float minDis   = 10;
                for(int i=0; i<circles.size(); i++) {
                    circles[i].get()->addAttractionPoint(fingerFinder.fingerPoints[0].x * adjustX,fingerFinder.fingerPoints[0].y * adjustY, strength);
                    circles[i].get()->setDamping(damping, damping);
                }
            }
            
            fingersFound=fingerFinder.findFingers(contourFinder.blobs[0]);
            numFingerPoints= fingerFinder.fingerPoints.size();
        }
        
        if(contourFinder.nBlobs>1){
            edgeLine2.destroy();
            ofPoint p;
            for(int i = 0 ; i < contourFinder.blobs[1].pts.size();i += 10){
                p.x=contourFinder.blobs[1].pts[i].x * adjustX;
                p.y=contourFinder.blobs[1].pts[i].y * adjustY;
                drawing2.addVertex(p.x, p.y);
            }
            drawing2.simplify();
            drawing2.setClosed(true);
            
            edgeLine2.addVertexes(drawing2);
            edgeLine2.simplify();
            edgeLine2.setPhysics(0.0, 0.5, 0.5);
            edgeLine2.create(box2d.getWorld());
            
            SoundData * data = (SoundData*)edgeLine2.getData();
            drawing2.clear();
            fingersFound2=fingerFinder2.findFingers(contourFinder.blobs[1]);
        }
        
        if(contourFinder.nBlobs>2){
            edgeLine3.destroy();
            ofPoint p;
            for(int i = 0 ; i < contourFinder.blobs[2].pts.size();i += 10){
                p.x=contourFinder.blobs[2].pts[i].x * adjustX;
                p.y=contourFinder.blobs[2].pts[i].y * adjustY;
                drawing3.addVertex(p.x, p.y);
            }
            drawing3.simplify();
            drawing3.setClosed(true);
            edgeLine3.addVertexes(drawing3);
            edgeLine3.simplify();
            edgeLine3.setPhysics(0.0, 0.5, 0.5);
            edgeLine3.create(box2d.getWorld());
            SoundData * data = (SoundData*)edgeLine3.getData();
            drawing3.clear();
        }
        
        if(contourFinder.nBlobs>3){
            edgeLine4.destroy();
            ofPoint p;
            for(int i = 0 ; i < contourFinder.blobs[3].pts.size();i += 10){
                p.x=contourFinder.blobs[3].pts[i].x * adjustX;
                p.y=contourFinder.blobs[3].pts[i].y * adjustY;
                drawing4.addVertex(p.x, p.y);
            }
            drawing4.simplify();
            drawing4.setClosed(true);
            edgeLine4.addVertexes(drawing4);
            edgeLine4.simplify();
            edgeLine4.setPhysics(0.0, 0.5, 0.5);
            edgeLine4.create(box2d.getWorld());
            SoundData * data = (SoundData*)edgeLine4.getData();
            drawing4.clear();
        }
        
        if(contourFinder.nBlobs>4){
            edgeLine5.destroy();
            ofPoint p;
            for(int i = 0 ; i < contourFinder.blobs[4].pts.size();i += 10){
                p.x=contourFinder.blobs[4].pts[i].x * adjustX;
                p.y=contourFinder.blobs[4].pts[i].y * adjustY;
                drawing5.addVertex(p.x, p.y);
            }
            drawing5.simplify();
            drawing5.setClosed(true);
            edgeLine5.addVertexes(drawing5);
            edgeLine5.simplify();
            edgeLine5.setPhysics(0.0, 0.5, 0.5);
            edgeLine5.create(box2d.getWorld());
            SoundData * data = (SoundData*)edgeLine5.getData();
            drawing5.clear();
        }

        if(contourFinder.nBlobs > 0){
            if (fingerFinder.fingerPoints.size() <= 4) {
                fingerNUM1 = 0;
            }
            if (fingerFinder.fingerPoints.size() >= 5) {
                fingerNUM1 = 1;
                hand1finger1 = ofVec2f(fingerFinder.fingerPoints[5].x * adjustX, fingerFinder.fingerPoints[5].y * adjustY);
            }
            if (fingerFinder.fingerPoints.size() >= 25){
                fingerNUM1 = 2;
                hand1finger2 = ofVec2f(fingerFinder.fingerPoints[25].x * adjustX,fingerFinder.fingerPoints[25].y * adjustY);
            }
            if (fingerFinder.fingerPoints.size() >= 46){
                fingerNUM1 = 3;
                hand1finger3 = ofVec2f(fingerFinder.fingerPoints[46].x * adjustX,fingerFinder.fingerPoints[46].y * adjustY);
            }
            if (fingerFinder.fingerPoints.size() >= 55){
                fingerNUM1 = 4;
                hand1finger4 = ofVec2f(fingerFinder.fingerPoints[55].x * adjustX,fingerFinder.fingerPoints[55].y * adjustY);
            }
            if (fingerFinder.fingerPoints.size() >= 65){
                fingerNUM1 = 5;
                hand1finger5 = ofVec2f(fingerFinder.fingerPoints[65].x * adjustX,fingerFinder.fingerPoints[65].y * adjustY);
            }
        }else{
            fingerNUM1 = 0;
        }
        
        if(contourFinder.nBlobs > 1){
            if (fingerFinder2.fingerPoints.size() <= 4) {
                fingerNUM2 = 0;
            }
            if (fingerFinder2.fingerPoints.size() >= 5) {
                fingerNUM2 = 1;
                hand2finger1 = ofVec2f(fingerFinder2.fingerPoints[5].x * adjustX, fingerFinder2.fingerPoints[5].y * adjustY);
            }
            if (fingerFinder2.fingerPoints.size() >= 25){
                fingerNUM2 = 2;
                hand2finger2 = ofVec2f(fingerFinder2.fingerPoints[25].x * adjustX,fingerFinder2.fingerPoints[25].y * adjustY);
            }
            if (fingerFinder2.fingerPoints.size() >= 46){
                fingerNUM2 = 3;
                hand2finger3 = ofVec2f(fingerFinder2.fingerPoints[46].x * adjustX,fingerFinder2.fingerPoints[46].y * adjustY);
            }
            if (fingerFinder2.fingerPoints.size() >= 55){
                fingerNUM2 = 4;
                hand2finger4 = ofVec2f(fingerFinder2.fingerPoints[55].x * adjustX,fingerFinder2.fingerPoints[55].y * adjustY);
            }
            if (fingerFinder2.fingerPoints.size() >= 65){
                fingerNUM2 = 5;
                hand2finger5 = ofVec2f(fingerFinder2.fingerPoints[65].x * adjustX,fingerFinder2.fingerPoints[65].y * adjustY);
            }
        }else {
            fingerNUM2 = 0;
        }
    } //////kinect.isFrameNew()
    
    box2d.update();
    kinect.update();
    
    if(bContourLine){
        for (int i =0; i < contourFinder.blobs[0].nPts; i+= 40) {
            pathVerticesT.push_back(ofVec2f(contourFinder.blobs[0].pts[i].x*adjustX,
                                            contourFinder.blobs[0].pts[i].y*adjustY));
        }
        while(pathVerticesT.size() > 500) {
            pathVerticesT.pop_front();
        }
        
        pathLinesT.clear();
        for(unsigned int i = 0; i < pathVerticesT.size(); i++) {
            pathLinesT.addVertex(pathVerticesT[i]);
        }
    }
    
    bContourLine = fingerNUM1 > 3 ? true : false;  //指が３本以上の時、輪郭線モード
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofEnableAlphaBlending();
    
    /*if (bSound) { // 発音する時は下記のブレンドファンクでgrayImageを消す
        glBlendFunc(GL_DST_COLOR, GL_ZERO);
       // kinect.drawDepth(0, 0, ofGetWidth(), ofGetHeight());
    }
     */
    
    if (!bSound){ //発音しないときはgrayImageを描画
        ofPushStyle();
        ofSetColor(255);
        grayImage.draw(0,0, ofGetWidth(),ofGetHeight());
        ofPopStyle();
    }
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    if ( (fingerNUM1 == 1 || fingerNUM1 > 4) && bSound ) rotateSinewaveDraw(); // 指1本サイン波発音
    if ( (fingerNUM1 == 2 || fingerNUM1 > 4) && bSound ) lowSynthsDraw(); // 指2本の低音発音
    if ( bContourLine && bSound) contourLine(); // 指5本 手の輪郭線＋パルス波発音
    
    ofPolyline poly,poly2;
    
    float value = 1.0f;
    if (rotateSinewavesD .size() > 0) {
        value = rotateSinewavesD.back()->amp.getValue() * 50.0f;
    }
    
    for(int i=0; i<circles.size(); i++) {
        ofFill();
        SoundData * data = (SoundData*)circles[i].get()->getData();
        
        if(data && data->bHit){
            if ((ofGetFrameNum() % 2) == 0) {
                // ofLogNotice("ヒットした");
                synths.push_back(new MySynth(circles[i].get()->getPosition().x,circles[i].get()->getPosition().y));
                synths.back()->play();
            }
        }
        circles[i].get()->draw();
    }
    
    for(int i=0; i<boxes.size(); i++) {
        SoundData * data = (SoundData*)boxes[i].get()->getData();
        
        if(data && data->bBoxHit){
            if ((ofGetFrameNum() % 2) == 0) {
                //ofLogNotice("ボックスヒットした！！！");
                lowSynths.push_back(new myLowSynth(boxes[i].get()->getPosition().x,boxes[i].get()->getPosition().y));
                lowSynths.back()->play();
            }
        }
        boxes[i].get()->draw();
    }
    
    
    vector<MySynth*>::iterator it = synths.begin();
    while (it != synths.end())
    {
        MySynth *s = *it;
        s->draw();
        
        if (!s->isAlive())
        {
            it = synths.erase(it);
            delete s;
        }
        else
            it++;
    }
    
    if(contourFinder.nBlobs> 0 && fingerNUM1 > 0 && ! bContourLine && bSound){
        ofPushStyle();
        ofSetColor(80,80,255);
        edgeLine.draw();
        ofPopStyle();
    }
    
    if(contourFinder.nBlobs> 1 && fingerNUM1 > 0 && ! bContourLine && bSound){
        ofPushStyle();
        ofSetColor(255,80,80);
        edgeLine2.draw();
        ofPopStyle();
    }
    /*
    if (bText) {
        ofPushStyle();
        ofSetColor(255);
        ofDrawBitmapString("bAUOn= " +  ofToString(bAudioUniteOn[1]),300,100);
        ofDrawBitmapString("bAUOn= " +  ofToString(bAudioUniteOn[2]),300,110);
        ofDrawBitmapString("near= " +  ofToString(nearThreshold),350,120);
        ofDrawBitmapString("far= " + ofToString(farThreshold),350,130);
        ofDrawBitmapString("angle= " + ofToString(angle),350,140);
        ofDrawBitmapString("cF.nBlobs= " + ofToString(contourFinder.nBlobs),350,150);
        ofDrawBitmapString("fingerNUM1= " + ofToString(fingerNUM1),350,160);
        ofDrawBitmapString("fingerPoints1= " + ofToString(fingerFinder.fingerPoints.size()),350,170);
        ofDrawBitmapString("fingerNUM2= " + ofToString(fingerNUM1),350,180);
        ofDrawBitmapString("fingerPoints2= " + ofToString(fingerFinder2.fingerPoints.size()),350,190);
        ofDrawBitmapString("hand1finger1.x= " + ofToString(hand1finger1.x),350,200);
        ofDrawBitmapString("hand1finger1.y= " + ofToString(hand1finger1.y),350,210);
        ofPopStyle();
    }
     */
    
    if ( bAddBaloon && contourFinder.nBlobs > 0) {
        ofVec2f CF1centoroid = ofVec2f(contourFinder.blobs[0].centroid.x,contourFinder.blobs[0].centroid.y);
        if(fingerNUM1 > 0){
            ofVec2f dis = hand1finger1 - CF1centoroid;
            ofPushStyle();
            ofSetColor(255);
            ofVec2f lineEnd = ofVec2f(contourFinder.blobs[0].centroid.x+dis.x*2,contourFinder.blobs[0].centroid.y + dis.y*2);
            ofLine(hand1finger1.x,hand1finger1.y,lineEnd.x,lineEnd.y);
            ofPopStyle();
            
            /*
            float audioHeight = ofGetHeight() / 2.0f;
           // float phaseDiff = ofGetWidth() / float(bufSize);
            
            ofSetColor(0, 0, 255);
            ofNoFill();
            ofSetLineWidth(1);
        
            // 左チャンネルを描画
            ofBeginShape();
            for (int i = hand1finger1.x; i < hand1finger1.x+bufSize; i++) {
                //ofVertex(i * phaseDiff, hand1finger1.y  + left[i] * audioHeight);
                ofVertex(i , hand1finger1.y  + lineEnd.y);
            }
            ofEndShape();
             */
            
            for (int i = 1; i < contourFinder.nBlobs; i++) {
                fingerBaloonDist[i] = ofDist(contourFinder.blobs[i].centroid.x, contourFinder.blobs[i].centroid.y,lineEnd.x, lineEnd.y);
                
                if (fingerBaloonDist[i] < 20) {
//                        ofPushStyle();
//                        ofFill();
//                        ofSetColor(255);
//                        ofCircle(contourFinder.blobs[i].centroid, 20);
                    if(!bAudioUniteOn[i]){
                        source[i].play();
                        bAudioUniteOn[i] = true;
                    }
                }else if (fingerBaloonDist[i] >= 20 && !bAllBaloonSound)  {
                    if (bAudioUniteOn[i]) {
                        source[i].stop();
                        bAudioUniteOn[i] = false;
                    }
                }
            }
        }
    }
    
    ofPushStyle();
    ofSetColor(200);
    ofLine(pointer.x -10, pointer.y + 10, pointer.x + 10, pointer.y - 10);
    ofLine(pointer.x -10, pointer.y - 10, pointer.x + 10, pointer.y + 10);
    ofPopStyle();
    
    
    /*
     
     float audioHeight = ofGetHeight() / 2.0f;
     float phaseDiff = ofGetWidth() / float(bufSize);
     
     ofSetColor(0, 0, 255);
     ofNoFill();
     ofSetLineWidth(2);
     
     // 左チャンネルを描画
     ofBeginShape();
     for (int i = 0; i < bufSize; i++) {
     ofVertex(i * phaseDiff, audioHeight /2 + lAudio[i] * audioHeight);
     }
     ofEndShape();
     
     // 右チャンネル波形を描画
     ofBeginShape();
     for (int i = 0; i < bufSize; i++) {
     ofVertex(i * phaseDiff, audioHeight /2 * 3 + rAudio[i] * audioHeight);
     }
     ofEndShape();

     */
    /*
    if(bAllBaloonSound){
        if(contourFinder.nBlobs == 5){
            for (int i = 0; i < contourFinder.nBlobs; i++) {
                source[i+1].play();
                source[i+1].loop();
                bAllBaloonSound = false;
            }
        } else{
            for (int i = 1; i < contourFinder.nBlobs; i++) {
                source[i].play();
                source[i].loop();
                bAllBaloonSound = false;
            }
        }
    } else if(!bAddBaloon){
        if(contourFinder.nBlobs == 5){
            for (int i = 0; i < contourFinder.nBlobs; i++) source[i+1].stop();
        }else if(contourFinder.nBlobs > 1){
            for (int i = 1; i < contourFinder.nBlobs; i++) source[i].stop();
        }
    }
     */
    
    if(bAllBaloonSound){
        for (int i = 0; i < contourFinder.nBlobs; i++) {
            source[i+1].play();
            source[i+1].loop();
            bAllBaloonSound = false;
        }
    }
    /*else if(!bAllBaloonSound && bAddBaloon){
            for (int i = 0; i < contourFinder.nBlobs; i++) source[i+1].stop();
    }
     */
    
    if(bAddBaloon){
        ofPushStyle();
        {
            ofPushMatrix();
            {
                if (contourFinder.nBlobs > 1) {
                    tap1.getLeftWaveform(wave1, contourFinder.blobs[1].boundingRect.width, ofGetHeight()/4);
                    ofTranslate(contourFinder.blobs[1].centroid.x - contourFinder.blobs[1].boundingRect.width/2,
                                contourFinder.blobs[1].centroid.y - ofGetHeight()/8); //100, ofGetHeight()/4);
                    ofSetColor(255, 255, 0);
                    wave1.draw();
                }
            }
            ofPopMatrix();
            ofPushMatrix();
            {
                if (contourFinder.nBlobs > 2) {
                    tap2.getLeftWaveform(wave2, contourFinder.blobs[2].boundingRect.width, ofGetHeight()/4);
                    ofTranslate(contourFinder.blobs[2].centroid.x - contourFinder.blobs[2].boundingRect.width/2,
                                contourFinder.blobs[2].centroid.y - ofGetHeight()/8);
                    ofSetColor(0, 255, 255);
                    wave2.draw();
                 }
            }
            ofPopMatrix();
            ofPushMatrix();
            {
                if (contourFinder.nBlobs > 3) {
                    tap3.getLeftWaveform(wave3, contourFinder.blobs[3].boundingRect.width, ofGetHeight()/4);
                    ofTranslate(contourFinder.blobs[3].centroid.x - contourFinder.blobs[3].boundingRect.width/2,
                                contourFinder.blobs[3].centroid.y - ofGetHeight()/8);
                    ofSetColor(255, 0, 255);
                    wave3.draw();
                }
            }
            ofPopMatrix();
            ofPushMatrix();
            {
                if (contourFinder.nBlobs > 4) {
                    tap4.getLeftWaveform(wave4, contourFinder.blobs[4].boundingRect.width, ofGetHeight()/4);
                    ofTranslate(contourFinder.blobs[4].centroid.x - contourFinder.blobs[4].boundingRect.width/2,
                                contourFinder.blobs[4].centroid.y - ofGetHeight()/8);
                    ofSetColor(0, 0, 255);
                    wave4.draw();
                }
            }
            ofPopMatrix();
            
            if(contourFinder.nBlobs == 5){
                ofPushMatrix();
                {
                    tap5.getLeftWaveform(wave5, contourFinder.blobs[0].boundingRect.width, ofGetHeight()/4);
                    ofTranslate(contourFinder.blobs[0].centroid.x - contourFinder.blobs[0].boundingRect.width/2,
                                contourFinder.blobs[0].centroid.y - ofGetHeight()/8);
                    ofSetColor(255, 0, 0);
                    wave5.draw();
                }
                ofPopMatrix();
            }else if(contourFinder.nBlobs > 5){
                ofPushMatrix();
                {
                    if (contourFinder.nBlobs > 4) {
                        tap5.getLeftWaveform(wave5, contourFinder.blobs[5].boundingRect.width, ofGetHeight()/4);
                        ofTranslate(contourFinder.blobs[5].centroid.x - contourFinder.blobs[5].boundingRect.width/2,
                                    contourFinder.blobs[5].centroid.y - ofGetHeight()/8);
                        ofSetColor(255, 0, 0);
                        wave5.draw();
                    }
                }
                ofPopMatrix();
            }
        }
        ofPopStyle();
    }
    
    mainOutputSyphonServer.publishScreen();
}
//--------------------------------------------------------------
void ofApp::audioReceived(float *input, int bufferSize, int nChannels){
    //諸岡追記
    for (int i = 0; i < bufferSize; i++) {
        left[i] = input[i * 2];
        right[i] = input[i * 2 + 1];
    }
}

//--------------------------------------------------------------
void ofApp::contourLine(){
    
   // ofColor magentaPrint = ofColor::fromHex(0xec008c), yellowPrint = ofColor::fromHex(0xffee00);
    
    ofPushStyle();
    //glBlendFunc(GL_DST_COLOR, GL_ZERO);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    myLFPulseDraw();  //ugenのmyLFPulseをスタート
    
    ofColor cyan = ofColor::fromHex(0x11abec);

    ofSetColor(cyan,50);
    pathLinesT.draw();
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::myLFPulseDraw(){
    deque<myLFPulse*>::iterator it3 = myLFPulses.begin();
    while (it3 != myLFPulses.end())
    {
        myLFPulse *s = *it3;
        
        if (!s->isAlive())
        {
            it3 = myLFPulses.erase(it3);
            delete s;
        }
        else
            it3++;
    }
    
    if(contourFinder.nBlobs > 0 && bSound){
        if ((ofGetFrameNum() %10) == 4) {
            myLFPulses.push_back(new myLFPulse(hand1finger5.x,hand1finger5.y,3.0f));
            myLFPulses.back()->play();
        }
    }
    for(int i = 0;i < myLFPulses.size();i++) {
        myLFPulses[i]->release();
    }
}
//--------------------------------------------------------------
void ofApp::lowSynthsDraw(){
    deque<myLowSynth*>::iterator it2 = lowSynths.begin();
    while (it2 != lowSynths.end())
    {
        myLowSynth *s = *it2;
        s->draw();
        
        if (!s->isAlive())
        {
            it2 = lowSynths.erase(it2);
            delete s;
        }
        else
            it2++;
    }
    
    if(fingerNUM1 > 1){
        if ((ofGetFrameNum() %10) == 4){
            lowSynths.push_back(new myLowSynth(hand1finger2.x,hand1finger2.y));
            lowSynths.back()->play();
        }
    }
    
    for(int i = 0;i < lowSynths.size();i++) {
        lowSynths[i]->release();
    }
    
}
//--------------------------------------------------------------
void ofApp::rotateSinewaveDraw(){
    deque<rotateSinewave*>::iterator it1 = rotateSinewavesD.begin();
    while (it1 != rotateSinewavesD.end())
    {
        rotateSinewave *s = *it1;
        
        // glBlendFunc(GL_ONE,GL_ONE);
        s->rotation = 200;
        s->color1=(ofVec3f(253,217,217));
        s->color2=(ofVec3f(220,214,217));
        s->color3=(ofVec3f(253,217,217));
        s->radius =20 * (std::sin(counter) + 1);
        s->draw(hand1finger1.x * adjustX, hand1finger1.y * adjustY);
        
        if (!s->isAlive())
        {
            it1 = rotateSinewavesD.erase(it1);
            delete s;
        }
        else
            it1++;
    }
    
    if(fingerNUM1 > 0){
        if ((ofGetFrameNum() %10) == 4) {
            rotateSinewavesD.push_back(new rotateSinewave(hand1finger1.x * adjustX,hand1finger1.y * adjustY,
                                                          (std::sin(counter)/2 + 0.55f),4000,10000));
            rotateSinewavesD.back()->play();
         }
    }
    
    for(int i = 0;i < rotateSinewavesD.size();i++) {
        rotateSinewavesD[i]->release();
    }
}
//--------------------------------------------------------------
void ofApp::exit() {
    kinect.close();
}
//--------------------------------------------------------------
void ofApp::contactStart(ofxBox2dContactArgs &e) {
    if(e.a != NULL && e.b != NULL) {
        if(e.a->GetType() == b2Shape::e_edge && e.b->GetType() == b2Shape::e_circle) {   //e_edge   e_circle
            SoundData * aData = (SoundData*)e.a->GetBody()->GetUserData();
            SoundData * bData = (SoundData*)e.b->GetBody()->GetUserData();
            if(aData) aData->bHit = true;
            if(bData) bData->bHit = true;
        }
        
        if(e.a->GetType() == b2Shape::e_edge && e.b->GetType() == b2Shape::e_polygon) {   //e_edge   e_circle
            SoundData * aData = (SoundData*)e.a->GetBody()->GetUserData();
            SoundData * bData = (SoundData*)e.b->GetBody()->GetUserData();
            if(aData) aData->bBoxHit = true;
            if(bData) bData->bBoxHit = true;
        }
    }
}
//--------------------------------------------------------------
void ofApp::contactEnd(ofxBox2dContactArgs &e) {
    if(e.a != NULL && e.b != NULL) {
        SoundData * aData = (SoundData*)e.a->GetBody()->GetUserData();
        SoundData * bData = (SoundData*)e.b->GetBody()->GetUserData();
        if(aData){
            aData->bHit = false;
            aData->bBoxHit = false;
        }
        if(bData){
            bData->bHit = false;
            bData->bBoxHit = false;
        }
    }
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    if(key == 'r') reverve.showUI();
    if(key == 'f') filter.showUI();
    if(key == 'd') delay.showUI();


    if (key == 'c') {
        float r = ofRandom(4, 20);		// a random radius 4px - 20px
        ofPtr <ofxBox2dCircle> c = ofPtr <ofxBox2dCircle>(new ofxBox2dCircle);
        c.get()->setPhysics(1, 0.5, 0.9);
        c.get()->setup(box2d.getWorld(), mouseX, mouseY, ofRandom(10, 20));
        c.get()->setData(new SoundData());
        circles.push_back(c);
    }
    if (key == 'x') {
        float r = ofRandom(4, 20);		// a random radius 4px - 20px
        ofPtr <ofxBox2dRect> c = ofPtr <ofxBox2dRect>(new ofxBox2dRect);
        c.get()->setPhysics(1, 0.5, 0.9);
        c.get()->setup(box2d.getWorld(), mouseX, mouseY, 15+ofRandom(-2,2), 15+ofRandom(-2,2));
        c.get()->setData(new SoundData());
        boxes.push_back(c);
        
        /*boxes.push_back(ofPtr<ofxBox2dRect>(new ofxBox2dRect));
         boxes.back().get()->setPhysics(3.0, 0.53, 0.1);
         boxes.back().get()->setup(box2d.getWorld(), mouseX, mouseY, 15+ofRandom(-2,2), 15+ofRandom(-2,2));
         */
    }
    if (key == 'v') {
        if (circles.size() > 0) {
            circles.back()->destroy();
            circles.pop_back();
        }
    }
    if (key == 'z') {
        if (boxes.size() > 0){
            boxes.back()->destroy();
            boxes.pop_back();
        }
    }
    
    if(key == 't'){
        bText = !bText;
    }
    
    switch (key) {
        case 's':
            bSound = !bSound;
            break;
        case '>':
        case '.':
            farThreshold ++;
            if (farThreshold > 255) farThreshold = 255;
            break;
            
        case '<':
        case ',':
            farThreshold --;
            if (farThreshold < 0) farThreshold = 0;
            break;
            
        case '+':
        case '=':
            nearThreshold ++;
            if (nearThreshold > 255) nearThreshold = 255;
            break;
            
        case '-':
            nearThreshold --;
            if (nearThreshold < 0) nearThreshold = 0;
            break;
            
        case OF_KEY_UP:
            angle++;
            if(angle>30) angle=30;
            kinect.setCameraTiltAngle(angle);
            break;
            
        case OF_KEY_DOWN:
            angle--;
            if(angle<-30) angle=-30;
            kinect.setCameraTiltAngle(angle);
            break;
    }
}
//--------------------------------------------------------------
void ofApp::keyReleased(int key){
}
//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){
}
//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
}
//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
}
//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
}
//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
}
//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
}
//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
}