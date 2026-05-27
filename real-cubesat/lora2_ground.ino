#include <SPI.h>
#include <LoRa.h>

#define ss 5
#define rst 14
#define dio0 2

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("[GROUND] Constellation Control Ready!");
  Serial.println("Type 'start' to simulate Sat 1 flying over.\n");

  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(923E6)) {
    Serial.println("LoRa initialization failed!");
    while (1);
  }
}

void loop() {
  // รับคำสั่งจากคีย์บอร์ด
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input == "start") {
      Serial.println("[CMD] Sending START_PASS to Sat 1...");
      LoRa.beginPacket();
      LoRa.print("START_PASS");
      LoRa.endPacket();
    }
  }

  // รอฟังสัญญาณวิทยุ
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String msg = "";
    while (LoRa.available()) {
      msg += (char)LoRa.read();
    }
    
    // ถ้าได้รับแจ้งว่า Sat 1 บินผ่านเสร็จแล้ว
    if (msg.startsWith("HANDOVER:")) {
      int collected = msg.substring(9).toInt(); // แกะตัวเลขออกมา
      Serial.println("\n[GROUND] Received Handover from Sat 1!");
      Serial.println("Sat 1 collected data index: 1 to " + String(collected));
      
      int nextIndex = collected + 1; // คำนวณว่าจะให้ Sat 2 เริ่มที่เท่าไหร่
      Serial.println("[GROUND] Calculating trajectory for Sat 2...");
      Serial.println("[GROUND] Instructing Sat 2 to resume from index " + String(nextIndex) + "...");
      delay(3000); // จำลองเวลาที่ดาวเทียมดวงต่อไปกำลังบินมา
      
      // ยิงคำสั่งไปหาดาวเทียมดวงที่ 2
      LoRa.beginPacket();
      LoRa.print("RESUME:" + String(nextIndex));
      LoRa.endPacket();
    } 
    // ถ้าได้รับแจ้งว่า Sat 2 ทำงานเสร็จ
    else if (msg.startsWith("FINAL:")) {
      int finalCount = msg.substring(6).toInt();
      Serial.println("\n====================================");
      Serial.println("[GROUND] Constellation Mission Complete!");
      Serial.println("Sat 2 finished collection up to Array index: " + String(finalCount));
      Serial.println("====================================\n");
      Serial.println("Type 'start' for a new pass.\n");
    }
  }
}
