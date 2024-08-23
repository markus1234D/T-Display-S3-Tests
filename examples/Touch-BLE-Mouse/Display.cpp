#include "Display.h"

int joystickCenterX = -1;
int joystickCenterY = -1;
int xDistance = 0;
int yDistance = 0;

Display::Display(bool debug) {
  // Initialize the display
  this->debug = debug;
  if(debug == true && !Serial){
    Serial.begin(9600);
  }
}

void Display::println(String str){
  if(debug){
    Serial.println(str);
  }
}

void Display::dos(){
  println("dos");
}

void Display::calculate_min_max_coord(ScreenPos sp, int16_t *min_x, int16_t *max_x, int16_t *min_y, int16_t *max_y){
  switch (sp)
  {
  case ScreenPos::FULLSCREEN:
    *min_x = 0;
    *max_x = EXAMPLE_LCD_V_RES;
    *min_y = 0;
    *max_y = EXAMPLE_LCD_H_RES;
    break;
  case ScreenPos::FIRST_HALF:
    *min_x = 0;
    *max_x = EXAMPLE_LCD_V_RES;
    *min_y = 0;
    *max_y = EXAMPLE_LCD_H_RES / 2;
    break;
  case ScreenPos::SECOND_HALF:
    *min_x = 0;
    *max_x = EXAMPLE_LCD_V_RES;
    *min_y = EXAMPLE_LCD_H_RES / 2;
    *max_y = EXAMPLE_LCD_H_RES;
    break;
  
  default:
    println("Invalid ScreenPos");
    break;
  }
  
}

void Display::nextMode(){
  mode = (Mode)((mode + 1) % Mode::LEN);
  switch (mode)
  {
  case Mode::MOUSE:
    println("Mode: MOUSE");
    break;
  case Mode::SCROLLJOYSTICK:
    println("Mode: SCROLLJOYSTICK");
    break;
  case Mode::MOUSEJOYSTICK:
    println("Mode: MOUSEJOYSTICK");
    break;
  case Mode::CLICKER:
    println("Mode: CLICKER");
    break;
  default:
    println("Invalid Mode");
    break;
  }
}

void Display::touchSetup(){
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
    Display::scanDevices();

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

void Display::scanDevices(){
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

void Display::appendFunction(enum ScreenPos pos){
  int16_t min_x, max_x, min_y, max_y;
  calculate_min_max_coord(pos, &min_x, &max_x, &min_y, &max_y);

}

void Display::setFunctionArray(fp func1, fp func2, fp func3){
  this->funcArray[0] = func1;
  this->funcArray[1] = func2;
  this->funcArray[2] = func3;
}


void Display::print(String str){
  if(debug){
    Serial.print(str);
  }
}

void Display::begin(){

    bleMouse.begin();
    touchSetup();

    this->funcArray[0] = &Display::handleMouseJoystick;
    // this->funcArray[1] = &Display::handleScrollJoystick;
}

void Display::loop(){
  data[DataIdx::MILLIS][pasteIdx] = 0;
  data[DataIdx::TOUCHED][pasteIdx] = 0;
  data[DataIdx::X_idx][pasteIdx] = -1;
  data[DataIdx::Y_idx][pasteIdx] = -1;
  data[DataIdx::SCROLL_UP][pasteIdx] = 0;
  data[DataIdx::SCROLL_DOWN][pasteIdx] = 0;
  data[DataIdx::HOME_BUTTON][pasteIdx] = 0;

  uint16_t touched = this->touch.getPoint(&this->data[DataIdx::X_idx][this->pasteIdx], &this->data[DataIdx::Y_idx][this->pasteIdx], 1);
  // data[DataIdx::MILLIS][pasteIdx] = std::abs(int((int16_t)millis() - data[DataIdx::MILLIS][(pasteIdx-1)%42]));
  if (touched > 0) {
      data[DataIdx::TOUCHED][pasteIdx] = 1;

      // println(String(touched) + " points detected");
      // println("x: " + String(this->data[DataIdx::X_idx][this->pasteIdx]) + " y: " + String(this->data[DataIdx::Y_idx][this->pasteIdx]));
        if(data[DataIdx::TOUCHED][(pasteIdx-1)%42] == 0){ 
          Serial.println("First Touch");
        }
  }
  println(String(pasteIdx) + " ; MILLIS: " + String(data[DataIdx::MILLIS][pasteIdx]) + " ; TOUCHED: " + String(data[DataIdx::TOUCHED][pasteIdx]) + " ; X_idx: " + String(data[DataIdx::X_idx][pasteIdx]) + " ; Y_idx: " + String(data[DataIdx::Y_idx][pasteIdx]) + " ; SCROLL_UP: " + String(data[DataIdx::SCROLL_UP][pasteIdx]) + " ; SCROLL_DOWN: " + String(data[DataIdx::SCROLL_DOWN][pasteIdx]) + " ; HOME_BUTTON: " + String(data[DataIdx::HOME_BUTTON][pasteIdx]) + " ; ");
    int16_t min_x, max_x, min_y, max_y;
    switch (this->bereiche)
    {
    case 0:
      break;
    case 1:
      calculate_min_max_coord(FULLSCREEN, &min_x, &max_x, &min_y, &max_y);
      if(this->data[DataIdx::X_idx][this->pasteIdx] > min_x && this->data[DataIdx::X_idx][this->pasteIdx] < max_x &&
        this->data[DataIdx::Y_idx][this->pasteIdx] > min_y && this->data[DataIdx::Y_idx][this->pasteIdx] < max_y){
          (this->*funcArray[0])();
      }
      break;
    case 2:
      calculate_min_max_coord(FIRST_HALF, &min_x, &max_x, &min_y, &max_y);
      if(this->data[DataIdx::X_idx][this->pasteIdx] > min_x && this->data[DataIdx::X_idx][this->pasteIdx] < max_x &&
        this->data[DataIdx::Y_idx][this->pasteIdx] > min_y && this->data[DataIdx::Y_idx][this->pasteIdx] < max_y){
          (this->*funcArray[0])();
      }
      calculate_min_max_coord(SECOND_HALF, &min_x, &max_x, &min_y, &max_y);
      if(this->data[DataIdx::X_idx][this->pasteIdx] > min_x && this->data[DataIdx::X_idx][this->pasteIdx] < max_x &&
        this->data[DataIdx::Y_idx][this->pasteIdx] > min_y && this->data[DataIdx::Y_idx][this->pasteIdx] < max_y){
          (this->*funcArray[1])();
      }
      break;
    case 3:
      calculate_min_max_coord(FIRST_THIRD, &min_x, &max_x, &min_y, &max_y);
      if(this->data[DataIdx::X_idx][this->pasteIdx] > min_x && this->data[DataIdx::X_idx][this->pasteIdx] < max_x &&
        this->data[DataIdx::Y_idx][this->pasteIdx] > min_y && this->data[DataIdx::Y_idx][this->pasteIdx] < max_y){
          (this->*funcArray[0])();
      }
      calculate_min_max_coord(SECOND_THIRD, &min_x, &max_x, &min_y, &max_y);
      if(this->data[DataIdx::X_idx][this->pasteIdx] > min_x && this->data[DataIdx::X_idx][this->pasteIdx] < max_x &&
        this->data[DataIdx::Y_idx][this->pasteIdx] > min_y && this->data[DataIdx::Y_idx][this->pasteIdx] < max_y){
          (this->*funcArray[1])();
      }
      calculate_min_max_coord(THIRD_THIRD, &min_x, &max_x, &min_y, &max_y);
      if(this->data[DataIdx::X_idx][this->pasteIdx] > min_x && this->data[DataIdx::X_idx][this->pasteIdx] < max_x &&
        this->data[DataIdx::Y_idx][this->pasteIdx] > min_y && this->data[DataIdx::Y_idx][this->pasteIdx] < max_y){
          (this->*funcArray[2])();
      }
    default:
      break;
  }
  pasteIdx = (pasteIdx + 1) % 42;
}

void Display::handleMouse(void){
  if(data[DataIdx::TOUCHED][pasteIdx] == 1){
    if (data[DataIdx::TOUCHED][(pasteIdx-1)%42] == 0) {
      // first touch
    }
    else
    {
      bleMouse.move((int)data[DataIdx::X_idx][pasteIdx]-(int)data[DataIdx::X_idx][(pasteIdx-1)%42], (int)data[DataIdx::Y_idx][pasteIdx]-(int)data[DataIdx::Y_idx][(pasteIdx-1)%42], 0);
    }
  }
  else
  {
    bleMouse.release();
  }
}

void Display::handleMouseJoystick(){
  if(data[DataIdx::TOUCHED][pasteIdx] == 1){
    if (data[DataIdx::TOUCHED][(pasteIdx-1)%42] == 0) {
      // first touch
      joystickCenterX = data[DataIdx::X_idx][pasteIdx];
      joystickCenterY = data[DataIdx::Y_idx][pasteIdx];
    }
    else
    {
      xDistance = data[DataIdx::X_idx][pasteIdx] - joystickCenterX;
      yDistance = data[DataIdx::Y_idx][pasteIdx] - joystickCenterY;
      println("X Distance: " + String(xDistance) + " Y Distance: " + String(yDistance));
      bleMouse.move((int)xDistance/2, (int)yDistance/2, 0);
    }
  }
  else
  {
    joystickCenterX = -1;
    joystickCenterY = -1;
  }

}

void Display::handleScrollJoystick(){
  if(data[DataIdx::TOUCHED][pasteIdx] == 1){
    if (data[DataIdx::TOUCHED][(pasteIdx-1)%42] == 0) {
      // first touch
      joystickCenterX = data[DataIdx::X_idx][pasteIdx];
      joystickCenterY = data[DataIdx::Y_idx][pasteIdx];
    }
    else
    {
      xDistance = data[DataIdx::X_idx][pasteIdx] - joystickCenterX;
      yDistance = data[DataIdx::Y_idx][pasteIdx] - joystickCenterY;
      println("X Distance: " + String(xDistance) + " Y Distance: " + String(yDistance));
      if(yDistance > 0){
        bleMouse.move(0, 0, 1);
      }
      else if(yDistance < 0){
        bleMouse.move(0, 0, -1);
      }
    }
  }
  else
  {
    joystickCenterX = -1;
    joystickCenterY = -1;
  }
}

void Display::handleClicker(){
  if(data[DataIdx::TOUCHED][pasteIdx] == 1){
    if (data[DataIdx::TOUCHED][(pasteIdx-1)%42] == 0) {
      println("First Touch");
      if (data[DataIdx::X_idx][pasteIdx] > EXAMPLE_LCD_V_RES / 2) {
        bleMouse.press(MOUSE_RIGHT);
      } else {
        bleMouse.press(MOUSE_LEFT);
      }      
    }
    else
    {
      //move
    }
  }
  else
  {
    print("release ");
    if (data[DataIdx::X_idx][(pasteIdx-1)%42] > EXAMPLE_LCD_V_RES / 2) {
      bleMouse.release(MOUSE_RIGHT);
      println("Right");
    } else {
      bleMouse.release(MOUSE_LEFT);
      println("Left");
    }
  }
}