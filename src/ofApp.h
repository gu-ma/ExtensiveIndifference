#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxJSON.h"
#include "ofLogAudio.h"
#include "MotionAmplifier.h"
#include "ofxDatGui.h"
#include "ofxEasing.h"

class ofApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();
    
    // General
    string wrapString(string text, int width);
    void animateVariables();
    map<string, ofVec2f> animationVariables;  // not very clear to use a ofVec2f
    float initTime, maskAlpha;
    int step, newsCount;
    float resScale;
    bool showGui, animate, drawMesh;
    
    // text
    vector<string> newsKeywords, sentences, dates;
    string textBuffer, scrollingText;
    ofTrueTypeFont consolasScrolling, consolasTitle;
    
    // audio + video
    ofLogAudio log;  // speech
    ofVideoPlayer movie;
    ofSoundPlayer music;
    
    //
    MotionAmplifier motionAmplifier;

    // datGUI
    ofxDatGui* gui;
    void onToggleEvent(ofxDatGuiToggleEvent e);
    
    // ofParam
    ofParameter<float> blurAmount, windowSize, learningRate, strength;
    ofParameter<float> pbas, pmod, rate, volm, volMusic, volVideo;

    // unused
    ofFbo fbo;
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);    

};
