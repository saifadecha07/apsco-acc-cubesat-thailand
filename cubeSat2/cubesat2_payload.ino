#include <SPI.h>
#include <LoRa.h>
#include <SD.h>

// --- Configuration ---
// LoRa Pins
const int csPin = 10;          // LoRa radio chip select
const int resetPin = 9;        // LoRa radio reset
const int irqPin = 2;          // Hardware interrupt pin

// SD Card Pins
const int sdCsPin = 4;         // SD card chip select

const String dataFile = "data.txt";
const String myAddress = "SAT2";

unsigned long lastLogTime = 0;
const unsigned long logInterval = 5000; // Log data every 5 seconds

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("CubeSat 2: Store and Forward Node Initializing...");

  // Initialize SD Card
  if (!SD.begin(sdCsPin)) {
    Serial.println("SD Card initialization failed! Please insert SD card.");
    // In real satellite, don't block forever, maybe store in EEPROM/Flash instead
    while (1);
  }
  Serial.println("SD Card initialized.");

  // Initialize LoRa
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(433E6)) { // Set frequency (433MHz / 868MHz / 915MHz depending on your module)
    Serial.println("LoRa init failed. Check connections.");
    while (1);
  }
  Serial.println("LoRa initialized.");
}

void loop() {
  // 1. Data Collection Phase (Store)
  if (millis() - lastLogTime >= logInterval) {
    lastLogTime = millis();
    collectAndStoreData();
  }

  // 2. Command Listening Phase (Forward)
  checkForCommands();
}

void collectAndStoreData() {
  // Mock Sensor Data (Replace with real sensor reads e.g., BME280, MPU6050)
  float temp = 25.0 + random(-10, 10) / 10.0;
  float voltage = 3.7 + random(-2, 2) / 10.0;
  
  // Format: Timestamp, Temperature, Voltage
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

void checkForCommands() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String received = "";
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }
    
    Serial.println("Received Signal: " + received);
    
    // Ground Station or CubeSat 1 sends command "SAT2:DUMP" to request data
    if (received.indexOf("SAT2:DUMP") != -1) {
      Serial.println("Command accepted. Starting data downlink...");
      transmitStoredData();
    }
  }
}

void transmitStoredData() {
  File data = SD.open(dataFile);
  if (data) {
    while (data.available()) {
      String line = data.readStringUntil('\n');
      line.trim();
      if (line.length() > 0) {
        // Send line by line via LoRa
        LoRa.beginPacket();
        LoRa.print("FROM_SAT2:");
        LoRa.print(line);
        LoRa.endPacket();
        
        Serial.println("Sent: " + line);
        
        // Small delay to ensure the receiver processes the packet (adjust as needed)
        delay(150); 
      }
    }
    data.close();
    Serial.println("Downlink complete.");
    
    // Optional: Clear the file after a successful transmission to free up space
    // SD.remove(dataFile);
    // Serial.println("Old data cleared.");
  } else {
    Serial.println("No data to send or error opening file.");
  }
}
