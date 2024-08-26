#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

unsigned long lastMillis = 0;

const char* ssid = "ZenFone7 Pro_6535";
const char* password = "e24500606";

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void handleRoot() {
    String html = "<!DOCTYPE html>"
                  "<html lang=\"de\">"
                  "<head>"
                  "<meta charset=\"UTF-8\">"
                  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
                  "<title>ESP32 WebSocket</title>"
                  "</head>"
                  "<body>"
                  "<h1>ESP32 WebSocket Client</h1>"
                  "<input type='text' id='messageInput' placeholder='Nachricht eingeben'>"
                  "<button id='sendMessage'>Nachricht senden</button>"
                  "<div id='messages'></div>"
                  "<script>"
                  "const socket = new WebSocket('ws://' + window.location.hostname + ':81');"
                  "socket.onopen = function(event) {"
                  "  document.getElementById('messages').innerHTML += '<p>Connected to ESP32!</p>';"
                  "};"
                  "socket.onmessage = function(event) {"
                  "  document.getElementById('messages').innerHTML = '<p>' + event.data + '</p>';"
                  "};"
                  "document.getElementById('sendMessage').addEventListener('click', () => {"
                  "  const message = document.getElementById('messageInput').value;"
                  "  if (socket.readyState === WebSocket.OPEN) {"
                  "    socket.send(message);"
                  "  }"
                  "});"
                  "</script>"
                  "</body>"
                  "</html>";
    server.send(200, "text/html", html);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            Serial.printf("[%u] Connected!\n", num);
            webSocket.sendTXT(num, "Hello from ESP32!");
            break;
        case WStype_TEXT:
            Serial.printf("[%u] Received text: %s\n", num, payload);
            String receivedMessage = String((char*)payload);
            String response = "ESP32 received: " + receivedMessage;
            webSocket.sendTXT(num, response.c_str());
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
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("WebSocket server started.");

    // Start HTTP-Server
    server.on("/", handleRoot);
    server.begin();
    Serial.println("HTTP server started.");

    lastMillis = millis();
}

void loop() {
    server.handleClient();
    webSocket.loop();
    if(millis() - lastMillis > 100) {
        lastMillis = millis();
        webSocket.broadcastTXT("Hello from ESP32!");
    }
}