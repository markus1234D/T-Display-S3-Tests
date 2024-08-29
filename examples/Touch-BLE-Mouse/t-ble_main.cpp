#include "Display.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include "TouchHandler.h"

#ifdef __cplusplus
extern "C" {
    #endif
    uint8_t temprature_sens_read();
    #ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

TouchHandler touchHandler();
uint16_t xTouch, yTouch;

// const char *ssid = "SM-Fritz";
// const char *password = "47434951325606561069";
// const char* ssid = "ZenFone7 Pro_6535";
// const char* password = "e24500606";
const char* ssid = "FRITZ!Mox";
const char* password = "BugolEiz42";

WebServer server(80);

Display display(false);

void handleRoot() {
    server.send(200, "text/html", R"rawliteral(

<!DOCTYPE html>
<html lang="de">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Touchpad GUI</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            padding: 20px;
        }

        #screenContainer {
            display: flex;
            flex-direction: column; /* Ändert die Ausrichtung auf vertikal */
            width: 25%;
            height: 70vh;
            margin-top: 20px;
            border: 2px solid #333;
        }

        .screen-section {
            border: 1px solid #888;
            display: flex;
            justify-content: center;
            align-items: center;
            flex: 1;
            margin: 5px;
            padding: 20px;
        }

        label {
            margin-right: 10px;
        }

        select {
            margin-bottom: 20px;
        }

        button {
            margin-top: 20px;
            padding: 10px 20px;
            font-size: 16px;
        }

        #output {
            margin-top: 20px;
            font-weight: bold;
        }
    </style>
</head>
<body>
    <h1>Touchpad GUI</h1>

    <div>
        <label for="layoutSelect">Wähle Bildschirmaufteilung:</label>
        <select id="layoutSelect">
            <option value="1">1 Bereich</option>
            <option value="2">2 Bereiche</option>
            <option value="3">3 Bereiche</option>
        </select>
    </div>

    <div id="screenContainer">
        <!-- Dynamisch erzeugte Bereiche werden hier eingefügt -->
    </div>

    <button id="submitBtn">Auswahl anzeigen</button>

    <div id="output"></div>

    <script>
        document.getElementById('layoutSelect').addEventListener('change', function() {
            const screenContainer = document.getElementById('screenContainer');
            screenContainer.innerHTML = '';  // Bildschirm leeren

            const layoutValue = parseInt(this.value);

            for (let i = 0; i < layoutValue; i++) {
                // Erstellt einen neuen Bereich
                const section = document.createElement('div');
                section.className = 'screen-section';

                // Auswahl für Funktion
                const functionSelect = document.createElement('select');
                const options = ['Mouse', 'Scroll-Joystick', 'Joystick-Mouse', 'Clicker'];
                
                options.forEach(optionText => {
                    const option = document.createElement('option');
                    option.value = optionText.toLowerCase().replace(' ', '-');
                    option.textContent = optionText;
                    functionSelect.appendChild(option);
                });

                section.appendChild(functionSelect);

                // Fügt den Bereich in den Container ein
                screenContainer.appendChild(section);
            }
        });
        function functionName2Idx(name) {
            switch(name) {
                case "mouse":
                    return 0;
                case "joystick-mouse":
                    return 1;
                case "scroll-joystick":
                    return 2;
                case "clicker":
                    return 3;
                default:
                    return -1;
            }
        }
        document.getElementById('submitBtn').addEventListener('click', function() {
            const sections = document.querySelectorAll('.screen-section select');
            // let output = `{"len" : "${sections.length}",\n`;
            let output = "/functions?";

            sections.forEach((select, index) => {
                const idx = functionName2Idx(select.value);
                output += `Bereich${index}=${idx}&`;
            });
            output = output.slice(0, -1); 
            // output += "}";
            console.log(output);

            document.getElementById('output').textContent = output;
            fetch_command(output);
        });

        function fetch_command(command) {
            console.log("fetch_command: " + command);
            fetch(command)
        }

    </script>
</body>
</html>
    
    )rawliteral");
}

void initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void setup(){
    Serial.begin(9600);
    while (!Serial);
    delay(100);
    Serial.println("Starting BLE Mouse");
    display.begin();
    WiFi.disconnect();
    delay(100);
    initWiFi();
    server.begin();
    delay(100);


    server.on("/", handleRoot);
    server.on("favicon.ico", HTTP_GET, []() {
        server.send(404, "text/plain", "404: Not Found");
    });
    server.on("/functions", HTTP_GET, []() {
        Serial.println("Button pressed");
        // display.funcArray[0] = &Display::dos;

        int numArgs = server.args();
        Serial.print("Number of args: ");
        Serial.println(numArgs);
        display.bereiche = numArgs;
        
        for (int i=0; i< numArgs; i++){
            Serial.print(server.argName(i));
            Serial.print(" : ");
            String arg = server.arg(i);
            Serial.println(arg);

            if(server.argName(i) == "Bereich" + String(i)){
                int funcIdx = arg.toInt();
                Serial.print("Function index: ");
                Serial.println(funcIdx);
                switch (funcIdx)
                {
                case Display::Mode::MOUSE:
                    Serial.println("Mode: MOUSE");
                    display.funcArray[i] = &Display::handleMouse;
                    break;
                case Display::Mode::SCROLLJOYSTICK:
                    Serial.println("Mode: SCROLLJOYSTICK");
                    display.funcArray[i] = &Display::handleScrollJoystick;
                    break;
                case Display::Mode::MOUSEJOYSTICK:
                    Serial.println("Mode: MOUSEJOYSTICK");
                    display.funcArray[i] = &Display::handleMouseJoystick;
                    break;
                case Display::Mode::CLICKER:
                    Serial.println("Mode: CLICKER");
                    display.funcArray[i] = &Display::handleClicker;
                    break;
                default:
                    display.funcArray[i] = NULL;
                    break;
                }
            }
        }
        server.send(200, "text/html", "ok");
    });
}
unsigned long lastMillis = 0;
void loop(){
    display.loop();
    // touchHandler.readTouch(&xTouch, &yTouch);
    server.handleClient();

    delay(100);
    if (millis() - lastMillis > 1000) {
        lastMillis = millis();
        // Convert raw temperature in F to Celsius degrees
        Serial.print((temprature_sens_read() - 32) / 1.8);
        Serial.println(" C");
    }
}