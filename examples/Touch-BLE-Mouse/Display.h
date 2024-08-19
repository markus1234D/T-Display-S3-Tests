#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "TouchDrvCSTXXX.hpp"
#include "pin_config.h"
#include <BleMouse.h>
#include <cstdlib>



class Display {

public:
    Display(bool debug = true);  // Constructor
    enum ScreenPos{
    FULLSCREEN = 1,
    FIRST_HALF = 12,
    SECOND_HALF = 22,
    FIRST_THIRD = 13,
    SECOND_THIRD = 23,
    THIRD_THIRD = 33,
    };
    
    void dos(); 
    void begin();
    void loop();
    void appendFunction(enum ScreenPos pos);  //Listener
    
// private:
    bool debug;

    enum Mode {
        MOUSE,
        SCROLLJOYSTICK,
        MOUSEJOYSTICK,
        CLICKER,
        LEN
    } ;
    enum DataIdx {
        MILLIS,
        TOUCHED,
        X_idx,
        Y_idx,
        SCROLL_UP,
        SCROLL_DOWN,
        HOME_BUTTON,
        LEN_DATA
    };

    enum Mode mode = MOUSE;
    

    TouchDrvCSTXXX touch;
    int16_t x, y;

    BleMouse bleMouse;

    int16_t data[(int)DataIdx::LEN_DATA][42];
    uint32_t pasteIdx = 0;

    void println(String str);
    void print(String str);
    void calculate_min_max_coord(ScreenPos sp, int16_t *min_x, int16_t *max_x, int16_t *min_y, int16_t *max_y);
    void nextMode();
    void scanDevices();
    void touchSetup();

    void handleMouse(void);
    void handleScrollJoystick();
    void handleMouseJoystick();
    void handleClicker();

    typedef void (Display::*fp)(void);
    fp funcArray[1];
};


#endif // 