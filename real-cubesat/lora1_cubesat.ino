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

  Serial.println("[CONSTELLATION SIMULATOR: STORE & FORWARD]");
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

// ฟังก์ชันจำลองดาวเทียมบินผ่าน: เก็บข้อมูลเงียบๆ (Store) แล้วสาดข้อมูลตู้มเดียว (Forward)
void runSatellitePass(String satName) {
  Serial.println("\n[" + satName + "] Entering coverage zone! Starting STORE phase...");
  unsigned long startTime = millis();
  
  // สร้าง Array สำหรับเป็นบัฟเฟอร์เก็บข้อมูล (เก็บได้สูงสุด 50 ชุดต่อรอบ)
  TelemetryPacket buffer[50]; 
  int collectedCount = 0;

  // 1. PHASE: STORE (บินผ่าน 10 วินาที เก็บข้อมูลเงียบๆ ไม่ส่งวิทยุ)
  while (millis() - startTime < 10000 && collectedCount < 50) {
    // บันทึกข้อมูลลง Array (SD Card จำลอง)
    buffer[collectedCount].packet_id = dataIndex;
    buffer[collectedCount].timestamp = millis();
    buffer[collectedCount].battery_volts = 3.7 + ((float)random(0, 50) / 100.0);
    buffer[collectedCount].temperature_c = 20.0 + ((float)random(0, 150) / 10.0);

    Serial.printf("[%s] Reading Sensor... Stored in Buffer Index %d (Packet ID: %d)\n", 
                  satName.c_str(), collectedCount, dataIndex);
    
    dataIndex++;
    collectedCount++;
    delay(random(300, 800)); // สุ่มเวลาอ่านค่าเซ็นเซอร์
  }

  // 2. PHASE: FORWARD (Data Dump - ยิงข้อมูลทั้งหมดรวดเดียวลง Ground Station)
  Serial.printf("\n[%s] Pass complete. Initiating DATA DUMP (%d packets)...\n", satName.c_str(), collectedCount);
  
  for (int i = 0; i < collectedCount; i++) {
    LoRa.beginPacket();
    LoRa.write((uint8_t*)&buffer[i], sizeof(TelemetryPacket));
    LoRa.endPacket();
    
    // หน่วงเวลาเล็กน้อย (100ms) ให้วิทยุ Ground ฝั่งรับทำงานทัน และป้องกัน LoRa Module ค้าง
    delay(100); 
  }

  // 3. จบการส่ง Data Dump ส่งสัญญาณบอก Ground ว่าจบแล้ว
  Serial.println("[" + satName + "] Data Dump Complete. Sending HANDOVER/FINAL signal...");
  LoRa.beginPacket();
  if (satName == "SAT 1") LoRa.print("HANDOVER");
  else LoRa.print("FINAL");
  LoRa.endPacket();
}
