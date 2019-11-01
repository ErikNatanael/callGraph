#pragma once

#include "ofMain.h"

class Script {
public:
  glm::vec2 pos;
  float radius = 0;
  int scriptId;
  int numFunctions = 0;
  // url
  
  bool operator<(const Script& s) {
    return this->numFunctions > s.numFunctions;
  }
  
  bool operator==(const int id) {
    return this->scriptId == id;
  }
};

