#pragma once

#include "ofMain.h"

class Function {
public:
  int id;
  string name;
  string url;
  int scriptId;
  int calledTimes = 0;
  int lineNumber;
  int columnNumber;
  
  void print() {
    cout << calledTimes << " Name: " << name << " id: " << id << " scriptId: " << scriptId << " url: "<< url << " lineNumber: " << lineNumber << " columnNumber: " << columnNumber << endl;
  }
  
  bool operator<(const Function& f) {
    return this->calledTimes > f.calledTimes;
  }
};