#pragma once

#include "ofMain.h"
#include "ofxJSON.h"
#include "ofxPostProcessing.h"

#include "FunctionCall.h"
#include "Function.h"
#include "Script.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

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
		
		glm::vec2 findNewScriptPosition(float radius);
		void saveFrame();
		
		ofxJSONElement json;
		vector<FunctionCall> functionCalls;
		unordered_map<uint64_t, FunctionCall> callMap;
		unordered_map<uint32_t, Function> functionMap;
		vector<Script> scripts;
		
		uint64_t firstts = 1000000000000;
	  uint64_t lastts = 0;
		uint64_t timeWidth = 0;
		uint32_t numScripts = 0;
		uint32_t maxScriptId = 0;
		
		uint64_t timeCursor = 0;
		bool playing = false;
		
		ofFbo backgroundFbo;
		ofFbo foregroundFbo;
		ofFbo timelineFbo;
		ofImage grabImg;
		
		int WIDTH = 1920;
		int HEIGHT = 1080;
		ofxPostProcessing post;
		
};
