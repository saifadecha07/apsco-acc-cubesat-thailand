#include <SPI.h>
#include <LoRa.h>

#define ss 5
#define rst 14
#define dio0 2

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("💻 Ground Station (PC) Ready!");
  Serial.println("Type 'get' in the Serial Monitor to request data from CubeSat.\n");

  LoRa.setPins(ss, rst, dio0);
  
  if (!LoRa.begin(923E6)) {
    Serial.println("LoRa initialization failed!");
    while (1);
  }
}

void loop() {
  // 1. รับคำสั่งจากผู้ใช้ผ่านคีย์บอร์ด (พิมพ์ใน Serial Monitor ของคอมพิวเตอร์)
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input == "get") {
      Serial.println("📤 Sending 'GET_DATA' command to CubeSat...");
      LoRa.beginPacket();
      LoRa.print("GET_DATA");
      LoRa.endPacket();
      Serial.println("⏳ Waiting for response...");
    }
  }

  // 2. รอรับข้อมูลกลับมาจาก CubeSat
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    String incomingData = "";
    while (LoRa.available()) {
      incomingData += (char)LoRa.read();
    }
    
    Serial.println("\n📥 Signal from CubeSat Detected!");
    
    // 3. รอ 5 วินาทีก่อนแสดงผลขึ้นจอคอมพิวเตอร์
    Serial.println("⏳ Decoding... Waiting 5 seconds before display...");
    delay(5000);
    
    // 4. แสดงผลให้ผู้ใช้เห็นบนจอ
    Serial.println("\n====================================");
    Serial.println("💻 >>> TELEMETRY DATA ARRIVED <<< 💻");
    Serial.println("Signal Strength (RSSI): " + String(LoRa.packetRssi()) + " dBm");
    Serial.println("Payload: " + incomingData);
    Serial.println("====================================\n");
    Serial.println("Type 'get' to request data again.");
  }
}
