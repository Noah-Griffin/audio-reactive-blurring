#pragma once

#include "ofMain.h"
#include "fft.h"
#include "ofxGui.h"

#include <vector>

#define BUFFER_SIZE 256
#define NUM_WINDOWS 80

class ofApp : public ofBaseApp{
    
    public:
        
        void setup();
        void update();
        void draw();
        void exit();
        
        void keyPressed  (int key);
        void mouseMoved(int x, int y );
        void mouseDragged(int x, int y, int button);
        void mousePressed(int x, int y, int button);
        void mouseReleased();
        
        void audioReceived     (float * input, int bufferSize, int nChannels);
    
    void flash();
    void drawPlots();
    
    void jumpToKeyFrame();
    void jumpBackKeyFrame();
    void glitchFrame();
    
    float getAvgMag(float currentSampleMag, int sampleCount);
    
    int spikeCount;
    
    float spikeSum;
    float signalAvg;
    float impulseThreshold;
    float totalSignalSum;
    float currentMag;
    float totalMag;
    
    float projectionSize;
    
    
    int pixCount;

    int sampleCount;
    
    int n;
    int top;
    
    bool thresholdExceeded;

    std::vector<float> previousMags;
    std::vector<float> spikePlots;
    std::vector<float> avgPlots;
    std::vector<float> thresholdPlots;
    
    //video
    ofVideoPlayer video;
    ofImage image;
    ofFbo canvasFbo;
    
    //pixels
    ofPixels pix;
    ofTexture tex;
    
    //frames
    int currentFrame;
    int totalFrames;
    
    int frameStep;
    
    ofxPanel gui;
    ofxFloatSlider sensitivity;
    ofxFloatSlider decay;
    ofxFloatSlider noiseCancel;
    ofxFloatSlider timeWindow;
    ofxFloatSlider zoom;
    
    ofPolyline line;
    
    ofShader shaderBlurX;
    ofShader shaderBlurY;

    ofFbo fboBlurOnePass;
    ofFbo fboBlurTwoPass;
    
    
    int blurAmount;
    
    private:
        float * left;
        float * right;
        int     bufferCounter;
        fft     myfft;
        
        float magnitude[BUFFER_SIZE];
        float phase[BUFFER_SIZE];
        float power[BUFFER_SIZE];
    
        float prevBuffer[BUFFER_SIZE/2];
        
        float freq[NUM_WINDOWS][BUFFER_SIZE/2];
        float freq_phase[NUM_WINDOWS][BUFFER_SIZE/2];
};

//#endif


