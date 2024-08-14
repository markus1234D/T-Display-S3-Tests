#include <Wire.h>
#include <CST816_TouchLib.h>
#include <BleMouse.h>
#include "pin_config.h"
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>



using namespace MDO;

int xDistance = 0;
int yDistance = 0;
int xPrev = -1;
int yPrev = -1;

int xLowerCenter = EXAMPLE_LCD_H_RES / 4;
int yCenter = EXAMPLE_LCD_V_RES / 2;

enum Mode {
	MOUSE_GESTUREMOUSE,
	MOUSE_JOYSTICKMOUSE,
	LEN
} ;
enum Mode mode = MOUSE_GESTUREMOUSE;

BleMouse bleMouse;
CST816Touch oTouch;
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h


void nextMode() {
	mode = (Mode) ((mode + 1) % LEN);
	Serial.println("Mode: " + String((Mode) mode));
	if (mode == MOUSE_JOYSTICKMOUSE){
		analogWrite(PIN_LCD_BL, 50);
		tft.fillCircle(xLowerCenter, yCenter, 10, TFT_RED);
	}
	else {
		analogWrite(PIN_LCD_BL, 0);
	}
}

void handleJovstickMouse(int x, int y) {
	int xDistance = x - xLowerCenter;
	int yDistance = y - yCenter;
	// int xDistanceSign = xDistance > 0 ? 1 : -1;
	// int yDistanceSign = yDistance > 0 ? 1 : -1;
	// int stepSize = 1;
	bleMouse.move(yDistance/4, xDistance/4, 0);
}

void touchSetup(){
    Wire.begin(PIN_IIC_SDA, PIN_IIC_SCL);
	Wire.setClock(400000);	//For reliable communication, it is recommended to use a *maximum* communication rate of 400Kbps

	if (!oTouch.begin(Wire)) {
		Serial.println("Touch screen initialization failed..");
		while(true){
			delay(100);
		}
	}

	// if (!oTouch.setOperatingModeHardwareBased()) {
	// 	Serial.println("Set full hardware operational mode failed");
	// }
	if(!oTouch.setOperatingModeFast()) {
		Serial.println("Set fast operational mode failed");
	}

	if (oTouch.setNotifyOnMovement()) {
		oTouch.setMovementInterval(10);	//as example: limit to 10 per second (so 100 msec as interval)
	} else {
		//only available in 'fast'-mode
		Serial.println("Set notify on movement failed");
	}

	if (!oTouch.setNotificationsOnAllEvents()) {
		//only available in 'fast'-mode, provides events on touch and release of the screen
		Serial.println("Set notify on touch-and-release failed");
	}
			
	CST816Touch::device_type_t eDeviceType;
	if (oTouch.getDeviceType(eDeviceType)) {
		Serial.print("Device is of type: ");
		Serial.println(CST816Touch::deviceTypeToString(eDeviceType));
	}
	Serial.println("Touch screen initialization done");
}

void setup () {
    Serial.begin(115200);
    while (!Serial);
    bleMouse.begin();

    touchSetup();
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
	pinMode(PIN_LCD_BL, OUTPUT);
	analogWrite(PIN_LCD_BL, 255);
	tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
	tft.drawLine(0, EXAMPLE_LCD_V_RES/2, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES/2, TFT_WHITE);
}


void loop () {
	oTouch.control();
	// oTouch.printBuf();

	int x = -1;
	int y = -1;
	if (oTouch.hadTouch()) {
		oTouch.getLastTouchPosition(x, y);
        bleMouse.click();
	}
	else if (oTouch.hadGesture()) {	//note that a gesture typically starts with a touch. Both will be provided here.
		CST816Touch::gesture_t eGesture;
		oTouch.getLastGesture(eGesture, x, y);

		if (x > EXAMPLE_LCD_H_RES/2) {
			String gesture = CST816Touch::gestureIdToString(eGesture);
			Serial.print(gesture);
			int stepSize = 1;
			if (mode == MOUSE_GESTUREMOUSE) {
				if(gesture == "LEFT") {
					// UP
					bleMouse.move(0, 0, stepSize);
				} else if(gesture == "RIGHT") {
					// DOWN
					bleMouse.move(0, 0, -stepSize);
				} else if(gesture == "UP") {
					// RIGHT
					bleMouse.move(0, 0, 0, stepSize);
				} else if(gesture == "DOWN") {
					// LEFT
					bleMouse.move(0, 0, 0, -stepSize);
				} 
			} else if (mode == MOUSE_JOYSTICKMOUSE) {
				handleJovstickMouse(x, y);
			}
			if(gesture == "TOUCH_BUTTON") {
				nextMode();
			}

		} else {
			if(xPrev == -1 && yPrev == -1) {
				// Serial.println("First Touch");
				xPrev = x;
				yPrev = y;
			}
			xDistance = x - xPrev;
			// Serial.println("X Distance: " + String(xDistance));
			yDistance = y - yPrev;
			// Serial.println("Y Distance: " + String(yDistance));
			int xDistanceSign = xDistance > 0 ? 1 : -1;
			int yDistanceSign = yDistance > 0 ? 1 : -1;

			int moveX = (int) (xDistance * 2);
			// moveX = moveX * xDistanceSign;
			int moveY = (int) (yDistance * 2);
			// moveY = moveY * yDistanceSign;
			// Serial.println("Move X: " + String(moveX) + " Move Y: " + String(moveY));
			bleMouse.move(moveY, moveX, 0);

			// bleMouse.move(2*yDistance, 2*xDistance, 0);


			xPrev = x;
			yPrev = y;
		}


        // Serial.println("X_prev: " + String(xPrev) + " Y_prev: " + String(yPrev));


    } 
    else {
		if(xPrev != -1 && yPrev != -1) {
			// Serial.println("released at X: " + String(xPrev) + " Y: " + String(yPrev));
		}
        xPrev = -1;
        yPrev = -1;
    }
    delay(10);	
}
