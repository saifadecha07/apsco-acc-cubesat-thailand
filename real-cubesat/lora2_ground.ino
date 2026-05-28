#include <SPI.h>
#include <LoRa.h>

#define ss 5
#define rst 14
#define dio0 2

#pragma pack(push, 1)
struct TelemetryPacket {
  uint32_t packet_id;
  uint32_t timestamp;
  float battery_volts;
  float temperature_c;
};
#pragma pack(pop)

uint32_t lastReceivedId = 0; 
bool isMissionActive = false;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("[GROUND] Starlink Constellation Control Ready!");
  Serial.println("Type 'start' to begin real-time network pass.\n");

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
      lastReceivedId = 0; 
      isMissionActive = true;
      Serial.println("[CMD] Uplinking START_PASS to Constellation...");
      Serial.println("[GROUND] Sat 1 is entering coverage. Expecting real-time stream...");
      LoRa.beginPacket();
      LoRa.print("START_PASS");
      LoRa.endPacket();
    }
  }

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    if (packetSize == sizeof(TelemetryPacket)) {
      TelemetryPacket pkt;
      LoRa.readBytes((uint8_t*)&pkt, sizeof(pkt));
      
      if (pkt.packet_id > lastReceivedId + 1 && lastReceivedId != 0 && pkt.packet_id != 0) {
        Serial.printf(">> [NETWORK DROP] Missing packets! Expected %d, got %d <<\n", lastReceivedId + 1, pkt.packet_id);
      }

      Serial.printf("  -> [LIVE] ID: %-3d | Time: %lu ms | Batt: %.2fV | Temp: %.1fC | RSSI: %d dBm\n", 
            pkt.packet_id, pkt.timestamp, pkt.battery_volts, pkt.temperature_c, LoRa.packetRssi());
            
      lastReceivedId = pkt.packet_id; 
    } 
    else {
      String msg = "";
      while (LoRa.available()) msg += (char)LoRa.read();

      if (msg == "HANDOVER") {
         uint32_t nextId = lastReceivedId + 1;
         Serial.println("\n[GROUND] Sat 1 signal degrading. Initiating Seamless Handover to Sat 2...");
         
         // หน่วงเวลาแค่นิดเดียว (0.5วิ) เพื่อจำลอง Overlapping Coverage ของ Starlink (ไร้รอยต่อ)
         delay(500); 
         
         Serial.println("[CMD] Uplinking session data to Sat 2...");
         LoRa.beginPacket();
         LoRa.print("RESUME:" + String(nextId));
         LoRa.endPacket();
      } 
      else if (msg == "FINAL") {
         isMissionActive = false;
         Serial.printf("\n====================================\n");
         Serial.printf("[GROUND] Constellation Pass Complete!\n");
         Serial.printf("Total packets streamed continuously: %d\n", lastReceivedId);
         Serial.printf("====================================\n\nType 'start' for a new pass.\n");
      }
    }
  }
}
