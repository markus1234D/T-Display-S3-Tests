#include "Display.h"

Display::Display(bool debug = true) {
  // Initialize the display
  this->debug = debug;
  if(debug == true){
    Serial.begin(9600);
  }
}

void Display::println(String str){
  if(debug){
    Serial.println(str);
  }
}

