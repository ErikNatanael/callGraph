#include "ofApp.h"

void ofApp::saveFrame() {
  glReadBuffer(GL_FRONT);
  grabImg.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
  grabImg.save("screenshots/screenshot" + ofGetTimestampString() + ".png");
}

//--------------------------------------------------------------
void ofApp::setup() {
  WIDTH = ofGetWidth();
  HEIGHT = ofGetHeight();
  
  // must set makeContours to true in order to generate paths
  font.load("SourceCodePro-Regular.otf", 16, false, false, true);
  
  // *********************** LOAD AND PARSE JSON DATA
  std::string file = "profiles/software_art/scores/scripting_events.json";

  // Now parse the JSON
  bool parsingSuccessful = json.open(file);

  if (parsingSuccessful)
  {
      ofLogNotice("ofApp::setup JSON parsing successful");
  }
  else
  {
      ofLogNotice("ofApp::setup")  << "Failed to parse JSON" << endl;
  }
  ofLog() << json["events"];
  ofLog() << json["events"][3]["ts"];
  
  set<int> scriptIds; // set to see how many script ids there is

  if (json["events"].isArray())
  {
    const Json::Value& events = json["events"];
    for (Json::ArrayIndex i = 0; i < events.size(); ++i) {
      if(events[i]["name"] == "ProfileChunk"
        && events[i]["hasNodes"] == true) {
        uint64_t ts = events[i]["ts"].asLargestUInt();
        if(ts < firstts) firstts = ts;
        
        uint64_t chunkTime = 0;
        const Json::Value& timeDeltas = events[i]["timeDeltas"];
        for (Json::ArrayIndex k = 0; k < timeDeltas.size(); ++k) {
          chunkTime += timeDeltas[k].asInt();
        }
        const Json::Value& nodes = events[i]["nodes"];
        for (Json::ArrayIndex j = 0; j < nodes.size(); ++j) {
          FunctionCall tempCall;
          // TODO: more accurate division of the chunk time into functions
          // divide the chunk time evenly among the functions in the chunk
          tempCall.ts = ts + long(double(chunkTime)*0.001*j);
          tempCall.scriptId = nodes[j]["callFrame"]["scriptId"].asInt();
          tempCall.name = nodes[j]["callFrame"]["functionName"].asString();
          tempCall.id = nodes[j]["id"].asInt();
          tempCall.parent = nodes[j]["parent"].asInt();
          functionCalls.push_back(tempCall);
          callMap.insert({tempCall.ts, tempCall});
          scriptIds.insert(tempCall.scriptId);
          
          // create the associated script and store its url
          auto searchScript = find(scripts.begin(), scripts.end(), tempCall.scriptId);
          if(searchScript == scripts.end()) {
            Script tempScript;
            tempScript.scriptId = tempCall.scriptId;
            tempScript.url = nodes[j]["callFrame"]["url"].asString();
            scripts.push_back(tempScript);
          }
          
          // create the associated function
          auto search = functionMap.find(tempCall.id);
          if (search != functionMap.end()) {
              search->second.calledTimes += 1;
          } else {
             Function tempFunc;
             tempFunc.name = tempCall.name;
             tempFunc.id = tempCall.id;
             tempFunc.scriptId = tempCall.scriptId;
             tempFunc.lineNumber = nodes[j]["callFrame"]["lineNumber"].asInt();
             tempFunc.columnNumber = nodes[j]["callFrame"]["columnNumber"].asInt();
             functionMap.insert({tempFunc.id, tempFunc});
          }

          if(tempCall.scriptId > maxScriptId) maxScriptId = tempCall.scriptId;
          if(tempCall.ts > lastts) lastts = tempCall.ts;
        }
      }
    }
  }
  
  timeWidth = lastts - firstts;
  numScripts = scriptIds.size();
  cout << functionCalls.size() << " function calls registered" << endl;
  cout << scriptIds.size() << " script ids registered" << endl;
  cout << "first ts: " << firstts << endl;
  cout << "last ts: " << lastts << endl;
  cout << "time width: " << timeWidth << endl;
  timeCursor = firstts;
  
  // count how many functions are in each script
  for(auto& functionMapPair : functionMap) {
    // add the script to the scriptMap if it does not yet exist
    auto searchScript = find(scripts.begin(), scripts.end(), functionMapPair.second.scriptId);
    if(searchScript == scripts.end()) {
      Script tempScript;
      tempScript.scriptId = functionMapPair.second.scriptId;
      tempScript.numFunctions = 1;
      scripts.push_back(tempScript);
    } else {
      // else increase the number of scripts
      searchScript->numFunctions++;
    }
  }
  // sort scripts after number of functions to find a position for the biggest one first
  std::sort (scripts.begin(), scripts.end());
  
  // go through all scripts and calculate their radii and positions
  bool redoAllPositions = false;
  do {
    redoAllPositions = false;
    for(auto& script : scripts) {
      script.radius = 20 + (sqrt(script.numFunctions) * 5);
      //ofLogNotice() << "radius: " << script.radius;
      script.pos = findNewScriptPosition(script.radius);
      //ofLogNotice() << "pos: " << script.pos;
      if(script.pos == glm::vec2(-1, -1)) {
        ofLog() << "Could not find position, redo all positions!";
        // could not find a suitable position, redo all positions
        redoAllPositions = true;
        break; // break out of the for loop and start over
      }
      script.generatePaths(font);
    }
    if(redoAllPositions) {
      // set all positions back to 0, 0 for recalculation
      for(auto& script : scripts) {
        script.pos = glm::vec2(0, 0);
      }
    }
  } while(redoAllPositions);
  
  // set all function positions depending on the script position
  for(auto& pair : functionMap) {
    auto& func = pair.second;
    // get script position
    glm::vec2 scriptPos;
    float maxRadius = 0;
    auto searchScript = find(scripts.begin(), scripts.end(), func.scriptId);
    if(searchScript != scripts.end()) {
      scriptPos = searchScript->pos;
      maxRadius = searchScript->radius;
    }
    // add a polar coordinate distance to the script position
    float angle = ofRandom(0, TWO_PI*4);
    float radius = ofRandom(10, maxRadius-10);
    func.pos = glm::vec2(scriptPos.x + (cos(angle)*radius), scriptPos.y + (sin(angle)*radius));
    func.boundCenter = scriptPos;
    func.boundRadius = maxRadius;
  }
  
  // ***************************** INIT openFrameworks STUFF
  ofBackground(0);
  ofSetFrameRate(60);
  ofEnableAlphaBlending();
  ofSetBackgroundAuto(false); //disable automatic background redraw
  ofSetCircleResolution(100);
  backgroundFbo.allocate(WIDTH, HEIGHT, GL_RGBA32F);
  foregroundFbo.allocate(WIDTH, HEIGHT, GL_RGBA32F);
  timelineFbo.allocate(WIDTH, HEIGHT, GL_RGBA32F);
  canvasFbo.allocate(WIDTH, HEIGHT, GL_RGBA32F);
  resultFbo.allocate(WIDTH, HEIGHT, GL_RGBA32F);
  focusShader.init();
  
  // post.init(ofGetWidth(), ofGetHeight());
  // post.createPass<ZoomBlurPass>();
  // post.createPass<BloomPass>();
  //post.createPass<GodRaysPass>();
  
  timelineFbo.begin();
  ofBackground(0, 0);
  timelineFbo.end();
}

//--------------------------------------------------------------
void ofApp::update(){
  static float lastTime = 0;
  float dt = ofGetElapsedTimef()-lastTime;
  lastTime = ofGetElapsedTimef();
  
  // move functions
  if(playing) {
    for(auto& pair : functionMap) {
      pair.second.update(dt);
    }
    for(auto& script : scripts) {
      script.update(dt);
    }
  }
  
	//ofLog() << "fps: " << ofGetFrameRate();
}

//--------------------------------------------------------------
void ofApp::draw(){
  if(playing) {
    // draw the location of every function in the background
    backgroundFbo.begin();
      ofBackground(0);
      for(auto& callPair : callMap) {
        ofFill();
    
        // Find the function connected to the function call
        auto searchFunc = functionMap.find(callPair.second.id);
        if(searchFunc != functionMap.end()) {
          auto& func = searchFunc->second;
          func.drawShapeBackground();
        }
      }
      // draw every script
      for(auto& script: scripts) {
        script.draw();
        // font.drawString(to_string(script.scriptId), script.pos.x, script.pos.y-script.radius);
      }
    backgroundFbo.end();
    
    
    foregroundFbo.begin();
    ofSetColor(0, 1);
    ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
    foregroundFbo.end();
    
    // draw time cursor
    for(int i = 0; i < 10000; i++) {
      timeCursor += 1;
      auto search = callMap.find(timeCursor);
      if (search != callMap.end()) {
    
          //std::cout << "Found " << search->first << " " << search->second.name << '\n';
          ofSetColor(255, 255);
          //ofDrawLine(0, (double(search->second.scriptId)/maxScriptId) * ofGetHeight(), cursorX, ofGetHeight()*0.5);
          // Find the function connected to the function call
          auto searchFunc = functionMap.find(search->second.id);
          if(searchFunc != functionMap.end()) {
            auto& func = searchFunc->second;
    
            // find the parent function
            auto searchParentFunc = functionMap.find(search->second.parent);
            if(searchParentFunc != functionMap.end()) {
              auto& parent = searchParentFunc->second;
              glm::vec2 acc = glm::normalize(parent.pos - func.pos)*20;
              func.acc = acc;
              func.activate();
              foregroundFbo.begin();
                func.drawForeground(parent);
              foregroundFbo.end();
              backgroundFbo.begin();
                func.drawLineBackground(parent);
              backgroundFbo.end();
            }
          }
    
      } else {}
    }
    int cursorX = ( double(timeCursor-firstts)/double(timeWidth) ) * ofGetWidth();
    timelineFbo.begin();
    ofSetColor(255, 255);
    ofDrawRectangle(0, HEIGHT*0.99, cursorX, HEIGHT);
    timelineFbo.end();
    
    if(timeCursor > lastts) {
      timelineFbo.begin();
      ofBackground(0, 0);
      timelineFbo.end();
      foregroundFbo.begin();
      ofBackground(0, 0);
      foregroundFbo.end();
      backgroundFbo.begin();
      ofBackground(0);
      backgroundFbo.end();
      timeCursor = firstts;
    }
  }
  
  // post.begin();
  canvasFbo.begin();
    ofSetColor(255, 255);
    backgroundFbo.draw(0, 0);
    foregroundFbo.draw(0, 0);
  canvasFbo.end();
  focusShader.render(canvasFbo, resultFbo);
  resultFbo.draw(0, 0);
  // post.end();
  
  timelineFbo.draw(0, 0);
  
  // Alpha needs to be cleared in order to accurately capture with OBS or grabScreen()
  // see: https://forum.openframeworks.cc/t/different-transparency-and-colours-on-screen-in-recording/32784
  ofClearAlpha(); 
}

glm::vec2 ofApp::findNewScriptPosition(float radius) {
  // choose a new position at random
  // check if it's at least n pixels from any previous position
  glm::vec2 newPos;
  bool isGood = true;
  float minDistance = radius  + 50;
  const int margin = 100;
  int numTries = 0;
  const int maxTries = 500;
  
  do {
    newPos = glm::vec2(ofRandom(margin + radius, WIDTH-margin-radius), ofRandom(margin+radius, HEIGHT-margin-radius));
    isGood = true;
    for(auto& script : scripts) {
      float distance = glm::distance(newPos, script.pos);
      distance -= script.radius;
      if(distance < minDistance) {
        isGood = false;
        ++numTries;
        if(numTries >= maxTries) {
          return glm::vec2(-1, -1); // signal that a position could not be found
        }
        break;
      }
    }
  } while(!isGood);
  
  return newPos;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
  if(key == ' ') {
    playing = !playing;
  } else if (key=='s') {
    saveFrame();
  }

}



//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
  for(auto& script : scripts) {
    // scripts cannot overlap so break if a match is found
    if(script.checkIfInside(glm::vec2(x, y))) break;
  }
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
