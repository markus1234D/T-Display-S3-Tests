#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "TouchDrvCSTXXX.hpp"
#include "pin_config.h"

class TouchHandler
{
    public:
    TouchHandler(){
        touchSetup();
    }
    TouchDrvCSTXXX touch;
    int16_t x, y;

    void touchSetup()
    {
        pinMode(PIN_POWER_ON, OUTPUT);
        digitalWrite(PIN_POWER_ON, HIGH);


    #if SENSOR_RST != -1
        pinMode(PIN_TOUCH_RES, OUTPUT);
        digitalWrite(PIN_TOUCH_RES, LOW);
        delay(30);
        digitalWrite(PIN_TOUCH_RES, HIGH);
        delay(50);
        // delay(1000);
    #endif

        // Search for known CSTxxx device addresses
        uint8_t address = 0x15;
        Wire.begin(PIN_IIC_SDA, PIN_IIC_SCL);

        touch.setPins(PIN_TOUCH_RES, PIN_TOUCH_INT);
        touch.begin(Wire, address, PIN_IIC_SDA, PIN_IIC_SCL);

        Serial.print("Model :"); Serial.println(touch.getModelName());

        // T-Display-S3 CST816 touch panel, touch button coordinates are is 85 , 160
        touch.setCenterButtonCoordinate(85, 360);

        touch.setHomeButtonCallback([](void *user_data) {
            Serial.println("Home key pressed!");
            // data[(int)DataIdx::HOME_BUTTON][pasteIdx] = 1;
        }, NULL);

        touch.enableAutoSleep();
        
        // Unable to obtain coordinates after turning on sleep
        // CST816T sleep current = 1.1 uA
        // CST226SE sleep current = 60 uA
        // touch.sleep();

        // Set touch max xy
        // touch.setMaxCoordinates(536, 240);

        // Set swap xy
        // touch.setSwapXY(true);

        // Set mirror xy
        // touch.setMirrorXY(true, false);
    }

    bool readTouch(uint16_t *x, uint16_t *y)
    {
        uint8_t touchNum = touch.getSupportTouchPoint();
        if (touchNum == 0) {
            *x = -1;
            *y = -1;
            return false;
        }
        uint16_t xArr[5], yArr[5];
        uint16_t touched = touch.getPoint(&xArr, &yArr);
        Serial.println("Touched: " + String(touched) + "x: " + String(xArr[0]) + " y: " + String(yArr[0]));
    }


};