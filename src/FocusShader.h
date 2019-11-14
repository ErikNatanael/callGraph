#pragma once

#include "ofMain.h"
#include "ofShader.h"
#include "ofxGui.h"

// this code is based on the ZoomBlurPass from ofxPostProcessing by satcy, http://satcy.net
// there is a license that might apply

class FocusShader {
public:
  
  ofShader shader;
  float centerX = 0;
  float centerY = 0;
  // float exposure = 0.48;
  // float decay = 0.9;
  // float density = .01;
  // float weight = 0.5;
  // float clamp = 1;
  
  ofParameter<float> exposure;
  ofParameter<float> decay;
  ofParameter<float> density;
  ofParameter<float> weight;
  ofParameter<float> clamp;

	ofxPanel gui;
  
  void init() {
    shader.load("shaders/focusShader/shader");
    gui.setup("focusShaderPanel"); // most of the time you don't need a name but don't forget to call setup
  	gui.add(exposure.set( "exposure", 0.48, 0.01, 1.0 ));
    gui.add(decay.set( "decay", 0.9, 0.9, 0.99999 ));
    gui.add(density.set( "density", 0.01, 0.001, 2.0 ));
    gui.add(weight.set( "weight", 0.5, 0.01, 1.0 ));
    gui.add(clamp.set( "clamp", 1.0, 0.1, 3.0 ));
  	
  }
  
  void render(ofFbo& readFbo, ofFbo& writeFbo)
  {
      writeFbo.begin();
      shader.begin();
      
      shader.setUniformTexture("tDiffuse", readFbo.getTextureReference(), 1);
      shader.setUniform1f("fX", centerX);
      shader.setUniform1f("fY", centerY);
      shader.setUniform1f("fExposure", exposure);
      shader.setUniform1f("fDecay", decay);
      shader.setUniform1f("fDensity", density);
      shader.setUniform1f("fWeight", weight);
      shader.setUniform1f("fClamp", clamp);
      shader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
      
      ofSetColor(255, 255);
      ofRect(0, 0, writeFbo.getWidth(), writeFbo.getHeight());
      
      shader.end();
      writeFbo.end();
  }
  
  void drawGUI() {
    gui.draw();
  }
  
};