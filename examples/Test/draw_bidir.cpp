#include "WiFi.h"
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "TFT_eSPI.h"
#include "pin_config.h"
#include "TouchDrvCSTXXX.hpp"


// const char *ssid = "SM-Fritz";
// const char *password = "47434951325606561069";
// const char* ssid = "ZenFone7 Pro_6535";
// const char* password = "e24500606";
const char* ssid = "FRITZ!Mox";
const char* password = "BugolEiz42";

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
TouchDrvCSTXXX touch;
uint16_t xTouch, yTouch;
unsigned long lastMillis = 0;

WebServer server(80);
WebSocketsServer webSocketServer = WebSocketsServer(81);
unsigned long millisLast = 0;


void touchSetup(){
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

String extractCommand(const String& input) {
    int pos = input.indexOf('?');
    if (pos != -1) {
        return input.substring(0, pos);
    } else {
        return input; // Wenn kein '?' gefunden wird, ist der ganze String der Command
    }
}

// Funktion, um die Argumentnamen und -werte zu extrahieren
int extractArgs(const String& input, std::vector<String>& argNames, std::vector<String>& args) {
    int pos = input.indexOf('?');
    if (pos == -1) return 0; // Falls kein '?' vorhanden ist, keine Argumente

    String query = input.substring(pos + 1);
    int start = 0;
    int end;
    int len = 0;

    while ((end = query.indexOf('&', start)) != -1) {
        String pair = query.substring(start, end);
        int equalPos = pair.indexOf('=');

        if (equalPos != -1) {
            len++;
            argNames.push_back(pair.substring(0, equalPos));
            args.push_back(pair.substring(equalPos + 1));
        }

        start = end + 1;
    }

    // Letztes Paar verarbeiten (nach dem letzten '&')
    String pair = query.substring(start);
    int equalPos = pair.indexOf('=');

    if (equalPos != -1) {
        len++;
        argNames.push_back(pair.substring(0, equalPos));
        args.push_back(pair.substring(equalPos + 1));
    }
    return len;
}


void handleRoot() {
    String html = R"rawliteral(
    
<!DOCTYPE html>
<html lang="de">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Zeichenfeld mit Reset</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
            margin: 0;
            background-color: #f0f0f0;
        }
        #canvas {
            border: 2px solid black;
            cursor: crosshair;
            image-rendering: pixelated; /* Pixelige Darstellung */
        }
        #controls {
            margin-top: 20px;
        }
        #coordinates {
            margin-top: 10px;
        }
    </style>
</head>
<body>
    <h1>Zeichnen mit der Maus</h1>
    <canvas id="canvas" width="170" height="320"></canvas>
    <div id="coordinates">X: 0, Y: 0</div>
    <div id="controls">
        <button id="resetButton">Zurücksetzen</button>
        <label for="brightness">Helligkeit:</label>
        <input type="range" id="brightness" min="0" max="255" value="128">

    </div>

    <script>
        function extractCommand(input) {
            const pos = input.indexOf('?');
            if (pos !== -1) {
                return input.substring(0, pos);
            } else {
                return input; // Wenn kein '?' gefunden wird, ist der ganze String der Command
            }
        }

        function extractArgs(input) {
            const argNames = [];
            const args = [];
            
            const pos = input.indexOf('?');
            if (pos === -1) return { argNames, args }; // Falls kein '?' vorhanden ist, keine Argumente

            const query = input.substring(pos + 1);
            const pairs = query.split('&');

            pairs.forEach(pair => {
                const [name, value] = pair.split('=');
                if (name && value) {
                    argNames.push(name);
                    args.push(value);
                }
            });

            return { argNames, args };
        }

        const canvas = document.getElementById('canvas');
        const ctx = canvas.getContext('2d');
        const coordinatesDiv = document.getElementById('coordinates');
        const resetButton = document.getElementById('resetButton');
        const brightnessInput = document.getElementById('brightness');

        let webDrawing = false;

        var ws = new WebSocket('ws://' + window.location.hostname + ':81');
        ws.onopen = function() {
            console.log("Hello, Server");
        };
        ws.onmessage = function (evt) {
            console.log("msg: " + evt.data);
            if (evt.data == "clear") {
                ctx.clearRect(0, 0, canvas.width, canvas.height);
            }
            const command = extractCommand(evt.data);
            const { argNames, args } = extractArgs(evt.data);
            console.log("Command: " + command);
            console.log("ArgNames: " + argNames);
            console.log("Args: " + args);
            if (command === "/coord") {
                const x = 170-parseInt(args[0]);
                const y = 320-parseInt(args[1]);
                
                if(webDrawing) {
                    ctx.lineTo(x, y);
                    ctx.stroke();
                } else {
                    webDrawing = true;
                    ctx.beginPath();
                    ctx.moveTo(x, y);
                }
                
            } else if (command === "clear") {
                ctx.clearRect(0, 0, canvas.width, canvas.height);
            } else if (command === "release") {
                webDrawing = false;
                console.log("release");
                
            }

        };
        ws.onclose = function() {
            console.log("Connection is closed...");
        };

        let drawing = false;

        // Linienbreite für größere "Pixel"
        ctx.lineWidth = 5;  // Erhöht die Linienstärke
        ctx.linecolor = "red"; // Linienfarbe
        // Maus-Position verfolgen und anzeigen
        canvas.addEventListener('mousemove', (e) => {
            if (!drawing) {
                return;
            }
            const rect = canvas.getBoundingClientRect();
            const x = e.clientX - rect.left;
            const y = e.clientY - rect.top;
            coordinatesDiv.textContent = `X: ${Math.round(x)}, Y: ${Math.round(y)}`;
            console.log("/coord?X=" + Math.round(x) + "&Y=" + Math.round(y));
            ws.send("/coord?X=" + Math.round(x) + "&Y=" + Math.round(y));
            if (drawing) {

                ctx.lineTo(x, y);
                ctx.strokeStyle = '#ff0000';    // Linienfarbe rot
                ctx.stroke();
            }
        });

        // Zeichnen beginnen
        canvas.addEventListener('mousedown', (e) => {
            drawing = true;
            ctx.beginPath();
            const rect = canvas.getBoundingClientRect();
            const pointX = e.clientX - rect.left;
            const pointY = e.clientY - rect.top;
            console.log("/coord?X=" + Math.round(pointX) + "&Y=" + Math.round(pointY));
            ws.send("/coord?X=" + pointX + "&Y=" + pointY);
            ctx.moveTo(pointX, pointY);
        });

        // Zeichnen beenden
        canvas.addEventListener('mouseup', () => {
            drawing = false;

        });

        // Zeichnung zurücksetzen
        resetButton.addEventListener('click', () => {
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            coordinatesDiv.textContent = `X: 0, Y: 0`;
            ws.send("clear");
        });

        brightnessInput.addEventListener("input", function() {
        setTimeout(() => {
            const value = brightnessInput.value;
            console.log("brightness?value=" + value);
            ws.send("brightness?value=" + value);
        }, 100
    );
    });
        function updateBrightness(value) {
            ws.send("brightness?value=" + value);
        }
    </script>
</body>
</html>




)rawliteral";
    server.send(200, "text/html", html);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
    case WStype_DISCONNECTED:
        Serial.printf("[%u] Disconnected!\n", num);
        break;
    case WStype_CONNECTED:
        Serial.printf("[%u] Connected!\n", num);
        webSocketServer.sendTXT(num, "Hello from ESP32!");
        break;
    case WStype_TEXT:
        Serial.printf("[%u] Received text: %s\n", num, payload);
        String receivedMessage = String((char*)payload);
        // String response = "ESP32 received: " + receivedMessage;
        // webSocketServer.sendTXT(num, receivedMessage.c_str());

        std::vector<String> argNames;
        std::vector<String> args;
        String command = extractCommand(receivedMessage);
        Serial.println("Command: " + command);
        int numArgs = extractArgs(receivedMessage, argNames, args);
        for (int i = 0; i < numArgs; i++) {
            Serial.println("Arg " + argNames[i] + ": " + args[i]);
        }
        if(command == "/coord"){
            if(numArgs == 2){
                int x = args[0].toInt();
                int y = 320-args[1].toInt();                // todo evtl.: 320-y
                tft.fillCircle(y, x, 3, TFT_RED);
            }
        }
        else if(command == "clear"){
            tft.fillScreen(TFT_BLACK);
        }
        else if(command == "brightness"){
            if(numArgs == 1){
                int brightness = args[0].toInt();
                analogWrite(PIN_LCD_BL, brightness);
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    
    touchSetup();
    pinMode(PIN_POWER_ON, OUTPUT);
    pinMode(PIN_LCD_BL, OUTPUT);

    digitalWrite(PIN_POWER_ON, HIGH);
    analogWrite(PIN_LCD_BL, 100);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Start WebSocket-Server
    webSocketServer.begin();
    webSocketServer.onEvent(webSocketEvent);
    Serial.println("WebSocket server started.");

    // Start HTTP-Server
    server.on("/", handleRoot);
    server.begin();
    Serial.println("HTTP server started.");

    lastMillis = millis();
}
bool touchWasPressed = false;
uint16_t oldX = 2000;
uint16_t oldY = 2000;

void loop() {
    server.handleClient();
    webSocketServer.loop();

    if(millis() - lastMillis > 50) {
        lastMillis = millis();
        // webSocketServer.broadcastTXT("Hello from ESP32!");
        uint8_t touchNum = touch.getSupportTouchPoint();
    // Serial.println("Touch number: " + String(touchNum));
        int16_t xArr, yArr;
        uint16_t touched = touch.getPoint(&xArr, &yArr, 1);
        if(touched){
            // Serial.println("Touch detected");
            if(!touchWasPressed){
                //press
                Serial.println("Touch pressed");
                touchWasPressed = true;
            }
            xTouch = round(xArr);
            yTouch = round(yArr);
            Serial.println("touch x: " + String(xTouch) + " y: " + String(yTouch));
            tft.fillCircle(/* 320- */yTouch, 170-xTouch, 5, TFT_WHITE);
            webSocketServer.broadcastTXT("/coord?X=" + String(xTouch) + "&Y=" + String(yTouch));
            
        } else {
            // Serial.println("Touch not ");
            if (touchWasPressed){
                //release
                Serial.println("Touch released");
                touchWasPressed = false;
                webSocketServer.broadcastTXT("release");
                touchWasPressed = false;
            }
        }
        oldX = xTouch;
        oldY = yTouch;
    }
}