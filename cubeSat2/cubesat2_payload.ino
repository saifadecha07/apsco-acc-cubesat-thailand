#include <SPI.h>
#include <LoRa.h>
#include <SD.h>

class CubeSatPayload {
private:
    // --- Configuration ---
    // LoRa Pins
    const int csPin = 10;
    const int resetPin = 9;
    const int irqPin = 2;

    // SD Card Pins
    const int sdCsPin = 4;

    const String dataFile = "data.txt";
    const String myAddress = "SAT2";

    unsigned long lastLogTime = 0;
    const unsigned long logInterval = 5000; // Log data every 5 seconds

    void collectAndStoreData() {
        // Mock Sensor Data
        float temp = 25.0 + random(-10, 10) / 10.0;
        float voltage = 3.7 + random(-2, 2) / 10.0;
        
        String dataString = String(millis()) + "," + String(temp) + "," + String(voltage);
        
        File data = SD.open(dataFile, FILE_WRITE);
        if (data) {
            data.println(dataString);
            data.close();
            Serial.println("Stored: " + dataString);
        } else {
            Serial.println("Error opening " + dataFile);
        }
    }

    void transmitStoredData() {
        File data = SD.open(dataFile);
        if (data) {
            while (data.available()) {
                String line = data.readStringUntil('\n');
                line.trim();
                if (line.length() > 0) {
                    LoRa.beginPacket();
                    LoRa.print("FROM_SAT2:");
                    LoRa.print(line);
                    LoRa.endPacket();
                    
                    Serial.println("Sent: " + line);
                    delay(150); 
                }
            }
            data.close();
            Serial.println("Downlink complete.");
        } else {
            Serial.println("No data to send or error opening file.");
        }
    }

public:
    CubeSatPayload() {}

    void begin() {
        Serial.begin(115200);
        while (!Serial);

        Serial.println("CubeSat 2: Store and Forward Node Initializing...");

        if (!SD.begin(sdCsPin)) {
            Serial.println("SD Card initialization failed! Please insert SD card.");
            while (1);
        }
        Serial.println("SD Card initialized.");

        LoRa.setPins(csPin, resetPin, irqPin);
        if (!LoRa.begin(433E6)) {
            Serial.println("LoRa init failed. Check connections.");
            while (1);
        }
        Serial.println("LoRa initialized.");
    }

    void update() {
        // 1. Data Collection Phase (Store)
        if (millis() - lastLogTime >= logInterval) {
            lastLogTime = millis();
            collectAndStoreData();
        }

        // 2. Command Listening Phase (Forward)
        int packetSize = LoRa.parsePacket();
        if (packetSize) {
            String received = "";
            while (LoRa.available()) {
                received += (char)LoRa.read();
            }
            
            Serial.println("Received Signal: " + received);
            
            if (received.indexOf("SAT2:DUMP") != -1) {
                Serial.println("Command accepted. Starting data downlink...");
                transmitStoredData();
            }
        }
    }
};

CubeSatPayload payload;

void setup() {
    payload.begin();
}

void loop() {
    payload.update();
}
