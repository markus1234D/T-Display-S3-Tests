#include "WiFi.h"
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "TFT_eSPI.h"
#include "pin_config.h"
#include "TouchDrvCSTXXX.hpp"


// const char *ssid = "SM-Fritz";
// const char *password = "47434951325606561069";
const char* ssid = "ZenFone7 Pro_6535";
const char* password = "e24500606";
// const char* ssid = "FRITZ!Box 5590 RR";
// const char* password = "92747535689889715932";

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
        <title>ESP32 WebSocket</title>
    </head>
    <body>
        <h1>ESP32 WebSocket Client</h1>
        <input type='text' id='messageInput' placeholder='Nachricht eingeben'>
        <button id='sendMessage'>Nachricht senden</button>
        <div id='messages'></div>
        
        <script>
            const socket = new WebSocket('ws://' + window.location.hostname + ':81');
            console.log('ws://' + window.location.hostname + ':81');
            socket.onopen = function(event) {
                document.getElementById('messages').innerHTML += '<p>Connected to ESP32!</p>';
            };
            socket.onmessage = function(event) {
                console.log(event.data);
                document.getElementById('messages').innerHTML = '<p>' + event.data + '</p>';
            };
            document.getElementById('sendMessage').addEventListener('click', () => {
                const message = document.getElementById('messageInput').value;
                if (socket.readyState === WebSocket.OPEN) {
                    socket.send(message);
                }
            });
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
            // webSocketServer.sendTXT(num, response.c_str());
            
            
            break;
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    

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

void loop() {
    server.handleClient();
    webSocketServer.loop();
    if(millis() - lastMillis > 100) {
        lastMillis = millis();
        webSocketServer.broadcastTXT("Hello from ESP32!");
    }
}