#include <SPI.h>
#include <LoRa.h>

#define ss 5
#define rst 14
#define dio0 2

// โครงสร้างข้อมูลต้องตรงกับฝั่งดาวเทียมเป๊ะๆ
#pragma pack(push, 1)
struct TelemetryPacket {
  uint32_t packet_id;
  uint32_t timestamp;
  float battery_volts;
  float temperature_c;
};
#pragma pack(pop)

uint32_t lastReceivedId = 0; // ตัวแปรสำหรับจำว่ารับ ID ล่าสุดเลขอะไรไป เพื่อเช็กของหาย

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
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input == "start") {
      lastReceivedId = 0; // รีเซ็ตระบบ
      Serial.println("[CMD] Sending START_PASS to Sat 1...");
      LoRa.beginPacket();
      LoRa.print("START_PASS");
      LoRa.endPacket();
    }
  }

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // 1. ถ้าขนาดแพ็คเกจเท่ากับขนาดของ Struct แปลว่าเป็น "ข้อมูลเซ็นเซอร์ (Telemetry)"
    if (packetSize == sizeof(TelemetryPacket)) {
      TelemetryPacket pkt;
      LoRa.readBytes((uint8_t*)&pkt, sizeof(pkt));
      
      // ระบบตรวจจับ Packet Loss (ข้อมูลหล่นหายกลางอากาศ)
      // ถ้า ID ที่รับมา มันมากกว่า ID ก่อนหน้าเกิน 1 สเต็ป แปลว่ามีอันที่หายไป!
      if (pkt.packet_id > lastReceivedId + 1 && lastReceivedId != 0 && pkt.packet_id != 0) {
        Serial.printf(">> [WARNING] PACKET LOSS DETECTED! Expected %d, but got %d <<\n", lastReceivedId + 1, pkt.packet_id);
      }

      // แสดงผลหน้าจอแบบ Real-time
      Serial.printf("[TELEMETRY] ID: %-3d | Time: %lu ms | Batt: %.2fV | Temp: %.1fC | RSSI: %d dBm\n", 
            pkt.packet_id, pkt.timestamp, pkt.battery_volts, pkt.temperature_c, LoRa.packetRssi());
            
      lastReceivedId = pkt.packet_id; // จำ ID ล่าสุดไว้
    } 
    // 2. ถ้าขนาดไม่เท่ากับ Struct แปลว่าเป็น "ข้อความคำสั่งสั่งการ (Command)"
    else {
      String msg = "";
      while (LoRa.available()) msg += (char)LoRa.read();

      if (msg == "HANDOVER") {
         uint32_t nextId = lastReceivedId + 1;
         Serial.printf("\n[GROUND] Sat 1 Pass Complete. Last ID successfully received: %d\n", lastReceivedId);
         Serial.printf("[GROUND] Instructing Sat 2 to resume from ID: %d in 3 seconds...\n", nextId);
         delay(3000);
         LoRa.beginPacket();
         LoRa.print("RESUME:" + String(nextId));
         LoRa.endPacket();
      } 
      else if (msg == "FINAL") {
         Serial.printf("\n====================================\n");
         Serial.printf("[GROUND] Constellation Mission Complete!\n");
         Serial.printf("Total packets collected and downlinked up to ID: %d\n", lastReceivedId);
         Serial.printf("====================================\n\nType 'start' for a new pass.\n");
      }
    }
  }
}
