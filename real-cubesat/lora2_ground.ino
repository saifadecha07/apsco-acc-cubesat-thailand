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
bool isReceivingDump = false;

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
      lastReceivedId = 0; 
      isReceivingDump = false;
      Serial.println("[CMD] Sending START_PASS to Sat 1...");
      Serial.println("[GROUND] Sat 1 is now in STORE phase (collecting silently for 10s)...");
      LoRa.beginPacket();
      LoRa.print("START_PASS");
      LoRa.endPacket();
    }
  }

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    if (packetSize == sizeof(TelemetryPacket)) {
      if (!isReceivingDump) {
        Serial.println("\n[DETECT] Incoming DATA DUMP burst detected! Receiving...");
        isReceivingDump = true;
      }

      TelemetryPacket pkt;
      LoRa.readBytes((uint8_t*)&pkt, sizeof(pkt));
      
      if (pkt.packet_id > lastReceivedId + 1 && lastReceivedId != 0 && pkt.packet_id != 0) {
        Serial.printf(">> [WARNING] PACKET LOSS DETECTED! Expected %d, but got %d <<\n", lastReceivedId + 1, pkt.packet_id);
      }

      Serial.printf("  -> [DUMP] ID: %-3d | Time: %lu ms | Batt: %.2fV | Temp: %.1fC | RSSI: %d dBm\n", 
            pkt.packet_id, pkt.timestamp, pkt.battery_volts, pkt.temperature_c, LoRa.packetRssi());
            
      lastReceivedId = pkt.packet_id; 
    } 
    else {
      String msg = "";
      while (LoRa.available()) msg += (char)LoRa.read();

      if (msg == "HANDOVER") {
         isReceivingDump = false;
         uint32_t nextId = lastReceivedId + 1;
         Serial.printf("\n[GROUND] Data Dump Complete! Last ID successfully received: %d\n", lastReceivedId);
         Serial.printf("[GROUND] Instructing Sat 2 to resume from ID: %d in 3 seconds...\n", nextId);
         delay(3000);
         Serial.println("[CMD] Sending RESUME to Sat 2...");
         Serial.println("[GROUND] Sat 2 is now in STORE phase (collecting silently for 10s)...");
         LoRa.beginPacket();
         LoRa.print("RESUME:" + String(nextId));
         LoRa.endPacket();
      } 
      else if (msg == "FINAL") {
         isReceivingDump = false;
         Serial.printf("\n====================================\n");
         Serial.printf("[GROUND] Constellation Mission Complete!\n");
         Serial.printf("Total packets collected via Data Dumps up to ID: %d\n", lastReceivedId);
         Serial.printf("====================================\n\nType 'start' for a new pass.\n");
      }
    }
  }
}
