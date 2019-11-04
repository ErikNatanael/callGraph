#pragma once

#include "ofMain.h"
#include "ofxPostProcessing.h"

#include "Timeline.h"
#include "FunctionCall.h"
#include "Function.h"
#include "FunctionDrawable.h"
#include "Script.h"
#include "ScriptDrawable.h"
#include "FocusShader.h"

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
		void exit();
		
		glm::vec2 findNewScriptPosition(float radius);
		void saveFrame();
		
		Timeline timeline;
		
		vector<ScriptDrawable> scripts;
		unordered_map<uint32_t, FunctionDrawable> functionMap;
		
		ofFbo backgroundFbo;
		ofFbo foregroundFbo;
		ofFbo timelineFbo;
		ofFbo canvasFbo;
		ofFbo resultFbo;
		ofImage grabImg;
		
		int WIDTH = 1920;
		int HEIGHT = 1080;
		FocusShader focusShader;
		
		ofTrueTypeFont font;
		
		list<TimelineMessage> messageFIFOLocal;
		
};
