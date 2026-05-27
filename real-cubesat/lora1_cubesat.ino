#include <SPI.h>
#include <LoRa.h>

#define ss 5
#define rst 14
#define dio0 2

// 1. สร้างโครงสร้างข้อมูล (Struct) แบบเดียวกับที่ใช้ในอุตสาหกรรม
#pragma pack(push, 1)
struct TelemetryPacket {
  uint32_t packet_id;    // หมายเลข Index
  uint32_t timestamp;    // เวลาที่เก็บข้อมูล
  float battery_volts;   // แรงดันแบตเตอรี่
  float temperature_c;   // อุณหภูมิ
};
#pragma pack(pop)

uint32_t dataIndex = 0;

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

// ฟังก์ชันหลักสำหรับจำลองดาวเทียมบินผ่านและยิงข้อมูลลงมาแบบ Real-time
void runSatellitePass(String satName) {
  Serial.println("\n[" + satName + "] Entering coverage zone! Starting DOWNLINK...");
  unsigned long startTime = millis();
  
  // บินผ่าน 10 วินาที
  while (millis() - startTime < 10000) {
    // สร้างแพ็คเกจข้อมูลจำลอง
    TelemetryPacket pkt;
    pkt.packet_id = dataIndex;
    pkt.timestamp = millis();
    pkt.battery_volts = 3.7 + ((float)random(0, 50) / 100.0);    // สุ่มไฟ 3.70 - 4.20V
    pkt.temperature_c = 20.0 + ((float)random(0, 150) / 10.0);   // สุ่มอุณหภูมิ 20.0 - 35.0C

    // ยิงข้อมูลเป็น Binary ก้อนเดียวลงไปที่ Ground
    LoRa.beginPacket();
    LoRa.write((uint8_t*)&pkt, sizeof(pkt));
    LoRa.endPacket();

    Serial.printf("[%s] Downlinking Packet ID: %d | Batt: %.2fV | Temp: %.1fC\n", 
                  satName.c_str(), pkt.packet_id, pkt.battery_volts, pkt.temperature_c);
    
    dataIndex++;
    delay(random(300, 800)); // หน่วงเวลาส่งเพื่อไม่ให้ช่องสัญญาณเต็มเกินไป
  }

  Serial.println("[" + satName + "] Leaving zone. Sending HANDOVER signal...");
  LoRa.beginPacket();
  if (satName == "SAT 1") LoRa.print("HANDOVER");
  else LoRa.print("FINAL");
  LoRa.endPacket();
}
