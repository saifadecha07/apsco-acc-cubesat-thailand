#include <SPI.h>
#include <LoRa.h>

#define ss 5
#define rst 14
#define dio0 2

int dataIndex = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("[CONSTELLATION SIMULATOR] Ready!");
  Serial.println("Waiting for Ground commands...");

  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(923E6)) {
    Serial.println("LoRa initialization failed!");
    while (1);
  }
  LoRa.setTxPower(20);
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String cmd = "";
    while (LoRa.available()) {
      cmd += (char)LoRa.read();
    }
    
    // --- สวมบทเป็นดาวเทียมดวงที่ 1 ---
    if (cmd == "START_PASS") {
      Serial.println("\n[SAT 1] Entering coverage zone! Starting collection...");
      dataIndex = 0;
      
      // บินผ่าน 10 วินาที (10,000 มิลลิวินาที)
      unsigned long startTime = millis();
      while (millis() - startTime < 10000) {
        dataIndex++;
        Serial.print("Collected Array Data #");
        Serial.println(dataIndex);
        delay(random(150, 400)); // สุ่มความเร็วในการเก็บข้อมูล
      }
      
      Serial.println("[SAT 1] Leaving coverage zone. Total collected: " + String(dataIndex));
      Serial.println("[SAT 1] Sending HANDOVER info to Ground...");
      
      LoRa.beginPacket();
      LoRa.print("HANDOVER:" + String(dataIndex));
      LoRa.endPacket();
    }
    // --- สวมบทเป็นดาวเทียมดวงที่ 2 ---
    else if (cmd.startsWith("RESUME:")) {
      dataIndex = cmd.substring(7).toInt(); // ดึงตัวเลขที่ต้องเริ่มเก็บต่อ
      Serial.println("\n[SAT 2] Entering coverage zone!");
      Serial.println("[SAT 2] Ground ordered to resume from Array #" + String(dataIndex));
      
      // บินผ่านอีก 10 วินาที
      unsigned long startTime = millis();
      while (millis() - startTime < 10000) {
        Serial.print("Collected Array Data #");
        Serial.println(dataIndex);
        dataIndex++;
        delay(random(150, 400));
      }
      
      Serial.println("[SAT 2] Leaving coverage zone. Final Array index: " + String(dataIndex - 1));
      Serial.println("[SAT 2] Sending FINAL report to Ground...");
      
      LoRa.beginPacket();
      LoRa.print("FINAL:" + String(dataIndex - 1));
      LoRa.endPacket();
    }
  }
}
