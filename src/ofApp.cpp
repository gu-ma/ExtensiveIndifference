#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    // General init
    showGui = false; // set to true to show GUI
    float r = 0.95; // ratio for the width of the window on the screen
    resScale = ofGetScreenWidth()*r/ofGetWidth();
    ofSetWindowShape((int)ofGetWidth()*resScale,(int)ofGetHeight()*resScale);
    ofSetWindowPosition((ofGetScreenWidth()-ofGetWidth())/2, (ofGetScreenHeight()-ofGetHeight())/2);
    ofSetFrameRate(24); // match framerate of the vid
    maskAlpha = 0;
    animate = true;
    drawMesh = false;
    newsCount = 8;
    newsKeywords = { "war", "carnage", "disaster", "rampage" };
    // newsKeywords = { "life", "technology", "business", "fight", "war", "dead", "disaster", "ISIS", "rampage" };
  
    // Animations values
    animationVariables["strength"].set(5,40);
    animationVariables["learningRate"].set(.1,1.15);
    animationVariables["blurAmount"].set(90,20);
    animationVariables["windowSize"].set(100,10);
    animationVariables["pbas"].set(130,20);
    animationVariables["pmod"].set(50,200);
    animationVariables["rate"].set(180,100);
    animationVariables["volm"].set(0.8,0.5);
    animationVariables["volMusic"].set(0,0.45);
    animationVariables["volVideo"].set(0.4,0.5);
    
    //

    //
    // load JSON(s)
    for (int c=0; c<newsKeywords.size(); c++) {
        string url = "http://api.nytimes.com/svc/search/v2/articlesearch.json?q=" + newsKeywords[c] + "&page=0&sort=newest&fl=headline,snippet,abstract,source,byline,pub_date&api-key=52697f63c9ade478ec6f2c7d71811aa6:17:61363877";
        // Now parse the JSON
        ofxJSONElement json;
        bool parsingSuccessful = json.open(url);
        if (parsingSuccessful) {
            int articlesCount = (json["response"]["docs"].size() < newsCount) ? json["response"]["docs"].size() : newsCount;
            for (Json::ArrayIndex i = 0; i < articlesCount; ++i) {
                string headline = json["response"]["docs"][i]["headline"]["main"].asString();
                string author = json["response"]["docs"][i]["byline"]["original"].asString();
                string date = json["response"]["docs"][i]["pub_date"].asString();
                string snippet = json["response"]["docs"][i]["snippet"].asString();
                sentences.push_back(snippet);
                dates.push_back(date);
            }
            
        } else {
            ofLogNotice("ofApp::setup") << "Failed to parse JSON.";
        }
        ofSleepMillis(1000);
    }
    
    //
    step = 0;
    textBuffer = ofJoinString(sentences, "\n");
    
    // audio log
    log.start();
    
    // video
    ofBackground(0);
    ofSetVerticalSync(true);
    movie.load("Clip11_3.mov");
    movie.setLoopState(OF_LOOP_NONE);
    movie.play();
    
    //
    motionAmplifier.setup(movie.getWidth(), movie.getHeight(), 2, .25);
    
    //ofParameters
    strength.set("Strength", animationVariables["strength"].x, -50, 50);
    learningRate.set("Learning Rate", animationVariables["learningRate"].x, 0, 2);
    blurAmount.set("Blur Amount", animationVariables["blurAmount"].x, 0, 100);
    windowSize.set("Window Size", animationVariables["windowSize"].x, 1, 200);
    pbas.set("pbas",animationVariables["pbas"].x,-500,500);
    pmod.set("pmod",animationVariables["pmod"].x,-500,500);
    rate.set("rate",animationVariables["rate"].x,1,600);
    volm.set("Speech",animationVariables["volm"].x,0,2);
    volMusic.set("Music",animationVariables["volMusic"].x,0,2);
    volVideo.set("Video",animationVariables["volVideo"].x,0,2);
    
    // datGUI
    gui = new ofxDatGui();
    //
    ofxDatGuiFolder* folder = gui->addFolder("options", ofColor::white);
    folder->addToggle("animate");
//    folder->addFRM();
    folder->addBreak();
    folder->addLabel("Motion");
    folder->addToggle("drawMesh");
    folder->addSlider(strength);
    folder->addSlider(learningRate);
    folder->addSlider(blurAmount);
    folder->addSlider(windowSize);
    folder->addBreak();
    folder->addLabel("Voice");
    folder->addSlider(pbas);
    folder->addSlider(pmod);
    folder->addSlider(rate);
    folder->addBreak();
    folder->addLabel("Sound");
    folder->addSlider(volm);
    folder->addSlider(volMusic);
    folder->addSlider(volVideo);
    gui->addHeader(" :::: ");
    //
    gui->setPosition(30*resScale, 30*resScale);
    gui->setTheme(new ofxDatGuiThemeCharcoal());
    //    new ofxDatGuiThemeSmoke(),
    //    new ofxDatGuiThemeMidnight(),
    //    new ofxDatGuiThemeCharcoal(),
    gui->setVisible( (showGui) ? true : false );
    // once the gui has been assembled, register callbacks to listen for component specific events //
    gui->onToggleEvent(this, &ofApp::onToggleEvent);

    //fonts
    ofTrueTypeFont::setGlobalDpi(96*resScale);
    consolasScrolling.load("consolas.ttf", 11, false);
    consolasScrolling.setLineHeight(17.0f*resScale);
    consolasScrolling.setLetterSpacing(1.037);
    consolasTitle.load("consolas.ttf", 50, false);

    
    //fbo
    fbo.allocate(ofGetWidth(), 1000, GL_RGBA);
    fbo.begin();
    ofClear(255,255,255,0);
    fbo.end();
    
    //sound
    music.load("sounds/2-20_Satisfaction_of_Oscillation.mp3");
    music.play();
//    music.setPositionMS(10000);
    
    //
    initTime = ofGetElapsedTimef();

}

//--------------------------------------------------------------
void ofApp::update(){

    animateVariables();

    ofFpsCounter fps;
    // ofSetWindowTitle(ofToString((int) ofGetFrameRate()) + " fps");
    ofSetWindowTitle(ofToString((int) ofGetElapsedTimef()) + " s");

    // update the sounds
    ofSoundUpdate();
    music.setVolume(volMusic);
    movie.setVolume(volVideo);
    
    // update shader
    motionAmplifier.setStrength(strength);
    motionAmplifier.setLearningRate(learningRate);
    motionAmplifier.setBlurAmount(blurAmount);
    motionAmplifier.setWindowSize(windowSize);
    
    //
    movie.update();
    if(movie.isFrameNew()) {
        motionAmplifier.update(movie);
    }

    // speech
    if (!log.isSpeaking && ofGetElapsedTimef() > (initTime+5.f) ) {
        
        // build speech settings
        string voice = "Tom";
        string pb = ofToString(pbas).substr(0,4);
        string pm = ofToString(pmod).substr(0,4);
        string ra = ofToString(rate).substr(0,4);
        string vl = ofToString(volm).substr(0,3);
        string msg = sentences[step];
        ofStringReplace(msg,"'"," ");
        string misc = "--interactive=green/black";
        log.LogAudio(voice, pb, pm, ra, vl, msg);
        
        // add text "header"
        if (step !=0) scrollingText.append("\n\n\n");
        scrollingText.append(dates[step] + "\n--------------\n");
        // step to the next article
        step ++;
        step %= sentences.size();
        
    }
    if (log.speechUpdate()) scrollingText.append(log.getCurrentWord() + " ");
  
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground(0);
    
    //
    ofSetupScreenOrtho(ofGetWidth(), ofGetHeight(), -100, +100);
    ofEnableDepthTest();
    ofPushMatrix();
    ofScale(resScale, resScale);
    motionAmplifier.draw(movie);
    if (drawMesh) motionAmplifier.drawMesh();
    ofPopMatrix();
    ofDisableDepthTest();
    
    //
    ofSetColor(0,maskAlpha);
    ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
    ofSetColor(255);
    
    //
    float padding = 30*resScale;
    string tempScrollingText = wrapString(scrollingText,ofGetWidth()/3);
    consolasScrolling.drawString(tempScrollingText, padding, ofGetHeight()-padding-consolasScrolling.stringHeight(tempScrollingText));
    string title = "Extensive \nIndifference";
    consolasTitle.drawString(title, padding, (ofGetHeight()-padding-consolasScrolling.stringHeight(tempScrollingText)-padding-consolasTitle.stringHeight(title)));
    
    // fbo.getTexture().drawSubsection(0, 0, ofGetWidth(), ofGetHeight(), 0, ofGetElapsedTimef(), ofGetWidth(), ofGetHeight());

}

//--------------------------------------------------------------
void ofApp::onToggleEvent(ofxDatGuiToggleEvent e)
{
    if (e.target->is("animate")) animate = !animate;
    if (e.target->is("drawMesh")) drawMesh = !drawMesh;
    cout << "onToggleEvent: " << e.target->getLabel() << " " << e.checked << endl;
}

//--------------------------------------------------------------
void ofApp::animateVariables(){

    auto duration = movie.getDuration();
    auto endTime = initTime + duration;
    auto now = ofGetElapsedTimef();
    auto duration1 = 15.f;
    auto initTime1 = endTime;
    auto endTime1 = initTime1 + duration1;
    
    if (animate) {
        strength = ofxeasing::map_clamp(now, initTime, endTime, animationVariables["strength"].x, animationVariables["strength"].y, &ofxeasing::linear::easeIn);
        learningRate = ofxeasing::map_clamp(now, initTime, endTime, animationVariables["learningRate"].x, animationVariables["learningRate"].y, &ofxeasing::linear::easeIn);
        blurAmount = ofxeasing::map_clamp(now, initTime, endTime, animationVariables["blurAmount"].x, animationVariables["blurAmount"].y, &ofxeasing::linear::easeIn);
        windowSize = ofxeasing::map_clamp(now, initTime, endTime, animationVariables["windowSize"].x, animationVariables["windowSize"].y, &ofxeasing::linear::easeIn);
        pbas = ofxeasing::map_clamp(now, initTime, endTime, animationVariables["pbas"].x, animationVariables["pbas"].y, &ofxeasing::linear::easeIn);
        pmod = ofxeasing::map_clamp(now, initTime, endTime, animationVariables["pmod"].x, animationVariables["pmod"].y, &ofxeasing::linear::easeIn);
        rate = ofxeasing::map_clamp(now, initTime, endTime, animationVariables["rate"].x, animationVariables["rate"].y, &ofxeasing::linear::easeIn);
        volm = ofxeasing::map_clamp(now, initTime, endTime, animationVariables["volm"].x, animationVariables["volm"].y, &ofxeasing::linear::easeIn);
        volMusic = ofxeasing::map_clamp(now, initTime, endTime, animationVariables["volMusic"].x, animationVariables["volMusic"].y, &ofxeasing::linear::easeIn);
        volVideo = ofxeasing::map_clamp(now, initTime, endTime, animationVariables["volVideo"].x, animationVariables["volVideo"].y, &ofxeasing::linear::easeIn);
        
        // dirty / quick :P
        if (ofGetElapsedTimef() > endTime) {
            
            maskAlpha = ofxeasing::map_clamp(now, initTime1, endTime1, 0, 255, &ofxeasing::quad::easeOut);
//            volm = ofxeasing::map_clamp(now, initTime1, endTime1, animationVariables["volm"].y, 0, &ofxeasing::linear::easeIn);
            rate = ofxeasing::map_clamp(now, initTime1, endTime1, animationVariables["rate"].y, 30, &ofxeasing::quad::easeOut);
            volMusic = ofxeasing::map_clamp(now, initTime1, endTime1, animationVariables["volMusic"].y, 0, &ofxeasing::quad::easeOut);
            volVideo = ofxeasing::map_clamp(now, initTime1, endTime1, animationVariables["volVideo"].y, 0, &ofxeasing::quad::easeOut);
        }
    }
}

//--------------------------------------------------------------
string ofApp::wrapString(string text, int width) {
    
    string typeWrapped = "";
    string tempString = "";
    vector <string> words = ofSplitString(text, " ");
    
    for(int i=0; i<words.size(); i++) {
        
        string wrd = words[i];
        
        // if we aren't on the first word, add a space
        if (i > 0) {
            tempString += " ";
        }
        tempString += wrd;
        
        int stringwidth = consolasScrolling.stringWidth(tempString);
        
        if(stringwidth >= width) {
            typeWrapped += "\n";
            tempString = wrd;		// make sure we're including the extra word on the next line
        } else if (i > 0) {
            // if we aren't on the first word, add a space
            typeWrapped += " ";  
        }  
        
        typeWrapped += wrd;  
    }  
    
    return typeWrapped;  
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
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
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
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

