#include <Wire.h>
#include <CST816_TouchLib.h>
#include <BleMouse.h>
#include "pin_config.h"

using namespace MDO;

int xDistance = 0;
int yDistance = 0;
int xPrev = -1;
int yPrev = -1;

BleMouse bleMouse;
CST816Touch oTouch;

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

}


void loop () {
	oTouch.control();
	
	if (oTouch.hadTouch()) {
		int x = 0;
		int y = 0;
		oTouch.getLastTouchPosition(x, y);
		
		Serial.print("Click received at: (");
		Serial.print(x);
		Serial.print(",");
		Serial.print(y);
		Serial.println(")");

        bleMouse.click();
	}
	
	if (oTouch.hadGesture()) {	//note that a gesture typically starts with a touch. Both will be provided here.
		CST816Touch::gesture_t eGesture;
		int x = 0;
		int y = 0;
		oTouch.getLastGesture(eGesture, x, y);

		// Serial.print("Gesture (");
		// Serial.print(CST816Touch::gestureIdToString(eGesture));
		// Serial.print(") received at: (");
		// Serial.print(x);
		// Serial.print(",");
		// Serial.print(y);
		// Serial.println(")");

        // Serial.println("X_prev: " + String(xPrev) + " Y_prev: " + String(yPrev));

	    if(xPrev == -1 && yPrev == -1) {
            Serial.println("First Touch");
            xPrev = x;
            yPrev = y;
        }
        xDistance = x - xPrev;
        // Serial.println("X Distance: " + String(xDistance));
        yDistance = y - yPrev;
        // Serial.println("Y Distance: " + String(yDistance));
        xPrev = x;
        yPrev = y;
        bleMouse.move(2*yDistance, 2*xDistance, 0);
    } 
    else {
        xPrev = -1;
        yPrev = -1;
    }
    delay(10);	
}
