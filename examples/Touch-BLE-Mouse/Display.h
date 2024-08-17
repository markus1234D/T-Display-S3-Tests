#ifndef DISPLAY_H
#define DISPLAY_H

 #include <Arduino.h>
// #include <Wire.h>
// #include <SPI.h>
// #include "TouchDrvCSTXXX.hpp"
// #include "pin_config.h"
// #include <BleMouse.h>



class Display {
public:
    Display(bool debug);  // Constructor
    // ~Display(); // Destructor
    void println(String str); 
//    void appendFunction();  //Listener
    
// private:
     bool debug;
//     enum Mode {
//         SCROLLJOYSTICK,
//         JOYSTICK_N_MOUSE,
//         LEN
//     } ;
//     enum DataIdx {
//         X_idx,
//         Y_idx,
//         x2xPrev_Distance_idx,
//         y2yPrev_Distance_idx,
//         TOUCHED,
//         LEN_DATA
//     };

//     enum Mode mode = JOYSTICK_N_MOUSE;

//     TouchDrvCSTXXX touch;
//     int16_t x[5], y[5];

//     BleMouse bleMouse;

//     int16_t data[(int)DataIdx::LEN_DATA][42];
};



#endif // DISPLAY_HPP