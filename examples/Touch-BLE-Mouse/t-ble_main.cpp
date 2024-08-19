#include "Display.h"

Display display(false);

void setup(){
    display.begin();
}

void loop(){
    display.loop();
    delay(50);
}