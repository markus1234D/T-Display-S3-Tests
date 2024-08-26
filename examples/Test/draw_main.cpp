// #include "WiFi.h"
// #include "ESPAsyncWebServer.h"
// #include "TFT_eSPI.h"
// #include "pin_config.h"
// #include "TouchHandler.h"
// #include "TouchDrvCSTXXX.hpp"

// // const char *ssid = "SM-Fritz";
// // const char *password = "47434951325606561069";
// const char* ssid = "ZenFone7 Pro_6535";
// const char* password = "e24500606";
 

// TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
// TouchDrvCSTXXX touch;
// uint16_t xTouch, yTouch;
// unsigned long millisLast = 0;

//  AsyncWebServer server(80);
// AsyncWebSocket ws("/");
// AsyncWebSocketClient * globalClient = NULL;

// void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
 
//   if(type == WS_EVT_CONNECT){
 
//     Serial.println("Websocket client connection received");
//     globalClient = client;
 
//   } else if(type == WS_EVT_DISCONNECT){
 
//     Serial.println("Websocket client connection finished");
//     globalClient = NULL;
 
//   }
// }

// void configWS(){
//   ws.onEvent(onWsEvent);
//   server.addHandler(&ws);
 
//   server.on("/html", HTTP_GET, [](AsyncWebServerRequest *request){
//     request->send(200, "/html", "text/html");
//   });
 
//   server.begin();
// }

// const char* html = R"rawliteral(

// <!DOCTYPE html>
// <html lang="de">
// <head>
//     <meta charset="UTF-8">
//     <meta name="viewport" content="width=device-width, initial-scale=1.0">
//     <title>Zeichenfeld mit Reset</title>
//     <style>
//         body {
//             font-family: Arial, sans-serif;
//             display: flex;
//             flex-direction: column;
//             align-items: center;
//             justify-content: center;
//             height: 100vh;
//             margin: 0;
//             background-color: #f0f0f0;
//         }
//         #canvas {
//             border: 2px solid black;
//             cursor: crosshair;
//             image-rendering: pixelated; /* Pixelige Darstellung */
//         }
//         #controls {
//             margin-top: 20px;
//         }
//         #coordinates {
//             margin-top: 10px;
//         }
//     </style>
// </head>
// <body>
//     <h1>Zeichnen mit der Maus</h1>
//     <canvas id="canvas" width="170" height="320"></canvas>
//     <div id="coordinates">X: 0, Y: 0</div>
//     <div id="controls">
//         <button id="resetButton">Zurücksetzen</button>
//     </div>
//     <div>
//         <p id = "display">Not connected</p>
//     </div>

//     <script>
//         const canvas = document.getElementById('canvas');
//         const ctx = canvas.getContext('2d');
//         const coordinatesDiv = document.getElementById('coordinates');
//         const resetButton = document.getElementById('resetButton');

//         var ws = new WebSocket("ws://192.168.100.137/");
//         ws.onopen = function() {
//             console.log("Hello, Server");
//         };
//         ws.onmessage = function (evt) {
//             console.log(evt.data);
//             if (evt.data == "clear") {
//                 ctx.clearRect(0, 0, canvas.width, canvas.height);
//             }
//         };
//         ws.onclose = function() {
//             console.log("Connection is closed...");
//         };

//         let drawing = false;

//         // Linienbreite für größere "Pixel"
//         ctx.lineWidth = 5;  // Erhöht die Linienstärke
//         ctx.linecolor = "red"; // Linienfarbe
//         // Maus-Position verfolgen und anzeigen
//         canvas.addEventListener('mousemove', (e) => {
//             if (!drawing) {
//                 return;
//             }
//             const rect = canvas.getBoundingClientRect();
//             const x = e.clientX - rect.left;
//             const y = e.clientY - rect.top;
//             coordinatesDiv.textContent = `X: ${Math.round(x)}, Y: ${Math.round(y)}`;
//             console.log("/coord?X=" + Math.round(x) + "&Y=" + Math.round(y));
//             fetch("/coord?X=" + Math.round(x) + "&Y=" + Math.round(y));
//             if (drawing) {
//                 ctx.lineTo(x, y);
//                 ctx.stroke();
//             }
//         });

//         // Zeichnen beginnen
//         canvas.addEventListener('mousedown', (e) => {
//             drawing = true;
//             ctx.beginPath();
//             const rect = canvas.getBoundingClientRect();
//             const pointX = e.clientX - rect.left;
//             const pointY = e.clientY - rect.top;
//             ctx.moveTo(pointY, pointX);
//             console.log("/coord?X=" + Math.round(pointX) + "&Y=" + Math.round(pointY));
            
//             fetch("/coord?X=" + pointX + "&Y=" + pointY);
//         });

//         // Zeichnen beenden
//         canvas.addEventListener('mouseup', () => {
//             drawing = false;
//         });

//         // Zeichnung zurücksetzen
//         resetButton.addEventListener('click', () => {
//             ctx.clearRect(0, 0, canvas.width, canvas.height);
//             coordinatesDiv.textContent = `X: 0, Y: 0`;
//             fetch("/clear");
//         });
//     </script>
// </body>
// </html>






// )rawliteral";

// void touchSetup(){
//     pinMode(PIN_POWER_ON, OUTPUT);
//     digitalWrite(PIN_POWER_ON, HIGH);


// #if SENSOR_RST != -1
//     pinMode(PIN_TOUCH_RES, OUTPUT);
//     digitalWrite(PIN_TOUCH_RES, LOW);
//     delay(30);
//     digitalWrite(PIN_TOUCH_RES, HIGH);
//     delay(50);
//     // delay(1000);
// #endif

//     // Search for known CSTxxx device addresses
//     uint8_t address = 0x15;
//     Wire.begin(PIN_IIC_SDA, PIN_IIC_SCL);

//     touch.setPins(PIN_TOUCH_RES, PIN_TOUCH_INT);
//     touch.begin(Wire, address, PIN_IIC_SDA, PIN_IIC_SCL);

//     Serial.print("Model :"); Serial.println(touch.getModelName());

//     // T-Display-S3 CST816 touch panel, touch button coordinates are is 85 , 160
//     touch.setCenterButtonCoordinate(85, 360);

//     // T-Display-AMOLED 1.91 Inch CST816T touch panel, touch button coordinates is 600, 120.
//     // touch.setCenterButtonCoordinate(600, 120);  // Only suitable for AMOLED 1.91 inch


//     // Depending on the touch panel, not all touch panels have touch buttons.
//     touch.setHomeButtonCallback([](void *user_data) {
//         Serial.println("Home key pressed!");
//         // data[(int)DataIdx::HOME_BUTTON][pasteIdx] = 1;
//     }, NULL);

//     touch.enableAutoSleep();
    
//     // Unable to obtain coordinates after turning on sleep
//     // CST816T sleep current = 1.1 uA
//     // CST226SE sleep current = 60 uA
//     // touch.sleep();

//     // Set touch max xy
//     // touch.setMaxCoordinates(536, 240);

//     // Set swap xy
//     // touch.setSwapXY(true);

//     // Set mirror xy
//     // touch.setMirrorXY(true, false);
// }
 
// void setup(){

//   Serial.begin(9600);
//   delay(1000);
// //   while (!Serial);
//   Serial.println("Starting BLE Mouse");

//   pinMode(PIN_POWER_ON, OUTPUT);
//   pinMode(PIN_LCD_BL, OUTPUT);

//   digitalWrite(PIN_POWER_ON, HIGH);
//   analogWrite(PIN_LCD_BL, 170);

//   tft.init();
//   tft.setRotation(1);
//   tft.fillScreen(TFT_BLACK);
 
//   WiFi.begin(ssid, password);
 
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(1000);
//     Serial.println("Connecting to WiFi..");
//   }
 
//   Serial.println(WiFi.localIP());
//   tft.println(WiFi.localIP());
//   tft.drawCentreString(String(WiFi.localIP()), 100, 85, 4);
 
//   configWS();

//   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
//     request->send(200, "text/html", html);
//   });
//   server.on("/coord", HTTP_GET, [](AsyncWebServerRequest *request){
//     int numArgs = request->args();
//     Serial.print("Number of args: ");
//     Serial.println(numArgs);
//     if(numArgs == 2){
//       Serial.print((*request).argName(0));
//       Serial.print(" : ");
//       String coordX = (*request).arg(size_t(0));
//       Serial.println(coordX);
//       Serial.print((*request).argName(1));
//       Serial.print(" : ");
//       String coordY = (*request).arg(size_t(1));
//       Serial.println(coordY);

//       tft.fillCircle(coordY.toInt(), 170-coordX.toInt(), 5, TFT_RED);
//     }
//     request->send(200, "text/html", "ok");
//   });
//   server.on("/clear", HTTP_GET, [](AsyncWebServerRequest *request){
//     tft.fillScreen(TFT_BLACK);
//     request->send(200, "text/html", "ok");
//   });
 
//   server.begin();
//   touchSetup();

//   millisLast = millis();
// }

// void handleWS(){
//   if(globalClient != NULL && globalClient->status() == WS_CONNECTED){
//     String randomNumber = String(random(0,20));
//     globalClient->text(randomNumber);
//   }
// }

// void loop(){
//   if(millis() - millisLast > 1000){
//     // Serial.println("Reading touch");
//     millisLast = millis();
//     // touchHandler.readTouch(&xTouch, &yTouch);
//     uint8_t touchNum = touch.getSupportTouchPoint();
//     // Serial.println("Touch number: " + String(touchNum));
//     if(touchNum > 0){
//         // Serial.println("Touch detected");
//         int16_t xArr, yArr;
//         uint16_t touched = touch.getPoint(&xArr, &yArr, 1);
//         xTouch = (xArr >1000) ? -1 : xArr;
//         yTouch = (yArr >1000) ? -1 : yArr;
//         // Serial.println("x: " + String(xTouch) + " y: " + String(yTouch));
//         tft.fillCircle(/* 320- */yTouch, 170-xTouch, 5, TFT_WHITE);
//     }

//     if(globalClient != NULL && globalClient->status() == WS_CONNECTED){
//       String randomNumber = String(random(0,20));
//       globalClient->text(randomNumber);
//    }
//   }
// }
