#include <SPI.h>
#include <LoRa.h>

#define ss 5
#define rst 14
#define dio0 2

// โครงสร้างข้อมูล (Struct)
#pragma pack(push, 1)
struct TelemetryPacket {
  uint32_t packet_id;
  uint32_t timestamp;
  float battery_volts;
  float temperature_c;
};
#pragma pack(pop)

uint32_t dataIndex = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("[STARLINK SIMULATOR: CONTINUOUS STREAM & RELAY]");
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
    while (LoRa.available()) cmd += (char)LoRa.read();
    
    if (cmd == "START_PASS") {
      dataIndex = 0;
      runSatellitePass("SAT 1");
    } 
    else if (cmd.startsWith("RESUME:")) {
      dataIndex = cmd.substring(7).toInt();
      runSatellitePass("SAT 2");
    }
  }
}

// ฟังก์ชันจำลองดาวเทียมบินผ่าน: สตรีมสด (Live Stream) ไม่มีการกักเก็บ
void runSatellitePass(String satName) {
  Serial.println("\n[" + satName + "] Entering coverage zone! Starting REAL-TIME STREAMING...");
  unsigned long startTime = millis();
  int streamedCount = 0;

  // REAL-TIME STREAMING PHASE (อ่านปุ๊บ ส่งปั๊บ ตลอดเวลา 10 วินาที)
  while (millis() - startTime < 10000) {
    TelemetryPacket pkt;
    pkt.packet_id = dataIndex;
    pkt.timestamp = millis();
    pkt.battery_volts = 3.7 + ((float)random(0, 50) / 100.0);
    pkt.temperature_c = 20.0 + ((float)random(0, 150) / 10.0);

    // ยิงข้อมูลทันที (Live Stream)
    LoRa.beginPacket();
    LoRa.write((uint8_t*)&pkt, sizeof(TelemetryPacket));
    LoRa.endPacket();

    Serial.printf("[%s] Streaming to Ground... Packet ID: %d\n", satName.c_str(), dataIndex);
    
    dataIndex++;
    streamedCount++;
    
    // หน่วงเวลาจำลองอัตราการส่งข้อมูลแบบต่อเนื่อง (ประมาณ 2 ครั้งต่อวินาที)
    delay(random(400, 700)); 
  }

  // จบการบินผ่าน ส่งสัญญาณ Seamless Handover ให้ดวงต่อไป
  Serial.println("[" + satName + "] Leaving coverage zone. Sending SEAMLESS HANDOVER signal...");
  LoRa.beginPacket();
  if (satName == "SAT 1") LoRa.print("HANDOVER");
  else LoRa.print("FINAL");
  LoRa.endPacket();
}
