#include <Arduino.h>
#include <vector>

// Funktion, um den Command zu extrahieren
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

void setup() {
    Serial.begin(115200);
    while(!Serial);
    delay(1000);

    // Beispiel-String
    String input = "command?argName1=arg1&argName2=arg2";
    Serial.println("Input: " + input);
    String command;
    std::vector<String> argNames;
    std::vector<String> args;

    // Command extrahieren
    command = extractCommand(input);
    Serial.println("Command: " + command);

    // Argumentnamen und -werte extrahieren
    int len = extractArgs(input, argNames, args);
    Serial.println("Number of Arguments: " + String(len));

    // Argumentnamen ausgeben
    Serial.print("Argument Names: ");
    for (const auto& name : argNames) {
        Serial.print(name + " ");
    }
    Serial.println();

    // Argumentwerte ausgeben
    Serial.print("Argument Values: ");
    for (const auto& value : args) {
        Serial.print(value + " ");
    }
    Serial.println();
}

void loop() {
    // Hier könntest du weitere Logik implementieren, falls nötig.
}
