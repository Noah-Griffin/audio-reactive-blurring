//VERSION TO BE EXHIBITED

#include "ofApp.h"
        
//--------------------------------------------------------------
void ofApp::setup(){
    blurAmount = 0;
    
    srand((unsigned int)time((time_t *)NULL));
    
    #ifdef TARGET_OPENGLES
        shaderBlurX.load("shadersES2/shaderBlurX");
        shaderBlurY.load("shadersES2/shaderBlurY");
    #else
        if(ofIsGLProgrammableRenderer()){
            shaderBlurX.load("shadersGL3/shaderBlurX");
            shaderBlurY.load("shadersGL3/shaderBlurY");
        }else{
            shaderBlurX.load("shadersGL2/shaderBlurX");
            shaderBlurY.load("shadersGL2/shaderBlurY");
        }
    #endif

//    image.load("img.jpg");
    
    fboBlurOnePass.allocate(ofGetScreenWidth(), ofGetScreenHeight());
    fboBlurTwoPass.allocate(ofGetScreenWidth(), ofGetScreenHeight());
    
    // 0 output channels,
    // 2 input channels
    // 44100 samples per second
    // BUFFER_SIZE samples per buffer
    // 4 num buffers (latency)
    
    ofBackground(0, 0, 0);
    projectionSize = 1;
    
    sampleCount = 0;
    totalMag = 0.0;
    thresholdExceeded = false;
    
    frameStep = 5;
    
    
    ofSoundStreamSetup(0, 2, 44100, BUFFER_SIZE, 4);
    left = new float[BUFFER_SIZE];
    right = new float[BUFFER_SIZE];
    
    for (int i = 0; i<BUFFER_SIZE; i++){
        prevBuffer[i] = 0;
    }

    for (int i = 0; i < NUM_WINDOWS; i++){
        for (int j = 0; j < BUFFER_SIZE/2; j++){
            freq[i][j] = 0;
        }
    }
    
    ofSetFrameRate(24);
    video.load("numb still colourised.mp4");
    video.setLoopState(OF_LOOP_NORMAL);
    
    video.play();
    
    video.setVolume(0);
//    video.setSpeed(0.4);
    
    totalFrames = video.getTotalNumFrames();
    
    //load pixel data from video into pixel array so that it can be later manipulated
    pix = video.getPixels();
    
    //fbo allows pixel manipulation to be drawn to the screen
    canvasFbo.allocate(ofGetScreenWidth(), (ofGetScreenWidth()/16) * 9, GL_RGBA, 5);
    pix.resize(ofGetScreenWidth(), (ofGetScreenWidth()/16) * 9);
    tex.loadData(pix);
    
    pixCount = pix.size();
//    ofSetColor(0);
    
    gui.setup();
    gui.add(sensitivity.setup("sensitivity", 10.0, 0.0, 50.0));
    gui.add(noiseCancel.setup("Noise Cancellation Threshold", 10.0, 0.0, 25.00));
    gui.add(timeWindow.setup("Sample window (time)", 22050.0, 0.0, 44100.0));
}


//--------------------------------------------------------------
void ofApp::update(){
    static int index=0;
    float avg_power = 0.0f;
    
    /* do the FFT    */
    myfft.powerSpectrum(0,(int)BUFFER_SIZE/2, left,BUFFER_SIZE,&magnitude[0],&phase[0],&power[0],&avg_power);
    
    /* start from 1 because mag[0] = DC component */
    /* and discard the upper half of the buffer */
    for(int j=1; j < BUFFER_SIZE/2; j++) {
        freq[index][j] = magnitude[j];
    }
    
    /* draw the FFT */
    for (int i = 1; i < 10; i++){
    //for (int i = 1; i < (int)(BUFFER_SIZE/2); i++){
        
        //if theres an increased magnitude at a specific freq since the previous buffer, add this magnitude to the spikeSum
        
        int mag = magnitude[i];
        
        //if mag of current band is less than the noise threshold, set mag to 0
        if (mag <= noiseCancel){
            mag = 0;
        }
        
        //if mag has increased from previous buffer, add mag to the total mag of spike
        if (mag > prevBuffer[i]){
            spikeSum += mag;
//            totalSignalSum += mag;
        }
        
        prevBuffer[i] = mag;
        
        spikeSum = spikeSum/spikeCount;
        
        //total mags regardless of spiking
        currentMag += mag;
        
        //running sample count - used later to get time window of samples
        sampleCount++;
        
    }
    
    signalAvg = getAvgMag(currentMag, sampleCount);
    
    impulseThreshold = signalAvg * sensitivity;
    
    spikePlots.push_back(spikeSum);
    thresholdPlots.push_back(impulseThreshold);
    avgPlots.push_back(signalAvg);
    
    video.update();
    pix = video.getPixels();
    
    int currentFrame = video.getCurrentFrame();
    
//    glitchFrame();
//    blurAmount =
//    std::cout << spikeSum;
//    blurAmount = ofMap(signalAvg, 0, 100, 0, 4);
    
    if (spikeSum > impulseThreshold && thresholdExceeded == false && currentFrame != totalFrames){
//        video.setSpeed(4.0);
//        glitchFrame();
//        for(int i=0; i<=10; i++){
//            blurAmount += 0.2;
//        }
//
        blurAmount = 0;

        thresholdExceeded = true;
        tex.loadData(pix);

    }

    else{
//        video.setSpeed(0.4);
        blurAmount = 0;
        thresholdExceeded = false;
        tex.loadData(pix);
    }

    spikeSum  = 0;
    currentMag = 0;
    
    tex.loadData(pix);

}

//--------------------------------------------------------------
void ofApp::draw(){
    
    float blur = blurAmount;
    
    //----------------------------------------------------------
    fboBlurOnePass.begin();
    
    shaderBlurX.begin();
    shaderBlurX.setUniform1f("blurAmnt", blur);

    tex.draw(0, 0, ofGetScreenWidth(), (ofGetScreenWidth()/16) * 9);
    canvasFbo.draw(0,0, ofGetScreenWidth(), (ofGetScreenWidth()/16) * 9);
    
    shaderBlurX.end();
    
    fboBlurOnePass.end();
    
    //----------------------------------------------------------
    fboBlurTwoPass.begin();
    
    shaderBlurY.begin();
    shaderBlurY.setUniform1f("blurAmnt", blur);
    
    fboBlurOnePass.draw(0, 0);
    
    shaderBlurY.end();
    
    fboBlurTwoPass.end();
    
    //----------------------------------------------------------
    ofSetColor(ofColor::white);
    fboBlurTwoPass.draw(0, 0);
    
    gui.draw();
}


float ofApp::getAvgMag(float currentSampleMag, int sampleCount){
    previousMags.push_back(currentSampleMag);
    
    totalMag += currentSampleMag;
    
    if (sampleCount >= timeWindow){
        float oldest = previousMags.front();
        totalMag -= oldest;
        
        previousMags.erase(previousMags.begin());
    }
    
    float avgMag = totalMag / timeWindow;
    
    return avgMag;
}


//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
    if (key == OF_KEY_UP && projectionSize < 1.0){
        projectionSize += 0.05;
        std::cout << "BIGGER" << std::endl;
    }
    
    if (key == OF_KEY_DOWN && projectionSize > 0){
        projectionSize -= 0.05;
        std::cout << "SMALLER" << std::endl;
    }
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
void ofApp::mouseReleased(){

}

//--------------------------------------------------------------
void ofApp::audioReceived     (float * input, int bufferSize, int nChannels){
    // samples are "interleaved"
    for (int i = 0; i < bufferSize; i++){
        left[i] = input[i*2];
        right[i] = input[i*2+1];
    }
    bufferCounter++;
}

void ofApp::exit(){
    ofSoundStreamStop();
}
