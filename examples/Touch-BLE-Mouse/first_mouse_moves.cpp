/**
 *
 * @license MIT License
 *
 * Copyright (c) 2022 lewis he
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file      TouchDrv_CSTxxx_GetPoint.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @date      2023-04-24
 *
 */
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "TouchDrvCSTXXX.hpp"
#include "pin_config.h"

TouchDrvCSTXXX touch;
int16_t x[5], y[5];


// BLE Mouse Stuff
#include <BleMouse.h>
BleMouse bleMouse;
// BLE Mouse Stuff End

enum GESTURE {
  NONE = 0x00,
  SWIPE_UP = 0x01,
  SWIPE_DOWN = 0x02,
  SWIPE_LEFT = 0x03,
  SWIPE_RIGHT = 0x04,
  SINGLE_CLICK = 0x05,
  DOUBLE_CLICK = 0x0B,
  LONG_PRESS = 0x0C

};

void scanDevices(void)
{
    byte error, address;
    int nDevices = 0;
    Serial.println("Scanning for I2C devices ...");
    for (address = 0x01; address < 0x7f; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (error == 0) {
            Serial.printf("I2C device found at address 0x%02X\n", address);
            nDevices++;
        } else if (error != 2) {
            Serial.printf("Error %d at address 0x%02X\n", error, address);
        }
    }
    if (nDevices == 0) {
        Serial.println("No I2C devices found");
    }
}

void setup()
{
    Serial.begin(115200);
    while (!Serial);
    bleMouse.begin();

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

    // Scan I2C devices
    scanDevices();

    Wire.beginTransmission(CST816_SLAVE_ADDRESS);
    if (Wire.endTransmission() == 0) {
        address = CST816_SLAVE_ADDRESS;
    }

    while (address == 0xFF) {
        Serial.println("Could't find touch chip!"); 
        delay(1000);
    }

    touch.setPins(PIN_TOUCH_RES, PIN_TOUCH_INT);
    touch.begin(Wire, address, PIN_IIC_SDA, PIN_IIC_SCL);

    Serial.print("Model :"); Serial.println(touch.getModelName());

    // T-Display-S3 CST816 touch panel, touch button coordinates are is 85 , 160
    touch.setCenterButtonCoordinate(85, 360);

    // T-Display-AMOLED 1.91 Inch CST816T touch panel, touch button coordinates is 600, 120.
    // touch.setCenterButtonCoordinate(600, 120);  // Only suitable for AMOLED 1.91 inch


    // Depending on the touch panel, not all touch panels have touch buttons.
    touch.setHomeButtonCallback([](void *user_data) {
        Serial.println("Home key pressed!");
    }, NULL);


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
int xDistance = 0;
int yDistance = 0;
int xPrev = 0;
int yPrev = 0;
void loop()
{

    uint8_t touched = touch.getPoint(x, y, touch.getSupportTouchPoint());
    if (touched) {
        // Serial.println("Touched: " + String(touched));
        Serial.println("X_prev: " + String(xPrev) + " Y_prev: " + String(yPrev));
        Serial.println("X: " + String(x[0]) + " Y: " + String(y[0]));
        if(xPrev == -1 && yPrev == -1) {
            Serial.println("First Touch");
            xPrev = (int)x[0];
            yPrev = (int)y[0];
        }
        xDistance = (int)x[0] - (int)xPrev;
        Serial.println("X Distance: " + String(xDistance));
        yDistance = y[0] - yPrev;
        Serial.println("Y Distance: " + String(yDistance));
        xPrev = x[0];
        yPrev = y[0];
        bleMouse.move(-2*xDistance, -2*yDistance, 0);
    } 
    else {
        xPrev = -1;
        yPrev = -1;
    }
    delay(100);
}



