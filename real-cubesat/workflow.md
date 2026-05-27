# LoRa Production-Grade Constellation Workflow

เอกสารนี้อธิบายลำดับการทำงานของระบบ "Constellation Simulator" ระดับอุตสาหกรรม (Production-Grade) ซึ่งจำลองการส่งข้อมูลเซ็นเซอร์ของดาวเทียมแบบ Binary Struct และดาวน์โหลด (Downlink) ข้อมูลแบบ Real-time พร้อมระบบตรวจจับ Packet Loss

---

## 1. การจัดโครงสร้างข้อมูล (Telemetry Data Structure)
เพื่อความสมจริงและประหยัดแบนด์วิดท์คลื่นวิทยุ ระบบไม่ได้ส่งข้อมูลเป็น String แต่แพ็คข้อมูลเป็น **C++ Binary Struct** ขนาดเล็ก:

```cpp
struct TelemetryPacket {
  uint32_t packet_id;    // หมายเลขแพ็คเกจ (Index เริ่มจาก 0)
  uint32_t timestamp;    // เวลาที่เก็บข้อมูล (มิลลิวินาที)
  float battery_volts;   // แรงดันแบตเตอรี่จำลอง (3.7V - 4.2V)
  float temperature_c;   // อุณหภูมิจำลอง (20C - 35C)
};
```

---

## 2. ลำดับการทำงานของระบบ (Sequence Diagram)

1.  **[Ground]** ผู้ใช้พิมพ์คำว่า `start` เพื่อเริ่มภารกิจ
2.  **[Ground -> Sat 1]** ยิงคำสั่ง `START_PASS` ขึ้นไป
3.  **[Sat 1]** ตอบรับคำสั่ง วนลูป 10 วินาที โดยในแต่ละรอบจะ:
    *   สร้าง `TelemetryPacket` (ใส่ค่า ID, เวลา, สุ่มอุณหภูมิ/แบตเตอรี่)
    *   ยิงข้อมูลไบนารีก้อนนี้ลงมาที่พื้นโลก **ทันที (Real-time Downlink)**
    *   บวกค่า ID เพิ่ม 1
4.  **[Ground]** รับก้อนไบนารี แปลงกลับเป็น Struct แล้วแสดงผลหน้าจอ
    *   *ระบบป้องกัน:* หากเลข `packet_id` ที่รับมากระโดดข้าม (เช่น จาก 5 ไป 7) จะขึ้นแจ้งเตือน **[WARNING] PACKET LOSS DETECTED!** ทันที
5.  **[Sat 1 -> Ground]** เมื่อหมด 10 วิ ส่งวิทยุข้อความ `HANDOVER`
6.  **[Ground]** รับคำสั่ง `HANDOVER` ดูว่าตัวเองได้รับ ID ล่าสุดถึงเลขอะไร แล้วบวก 1 เป็นเลขถัดไป (สมมติล่าสุดคือ 12 -> ต่อไปคือ 13)
7.  **[Ground]** หน่วงเวลาจำลอง 3 วินาทีรอ Sat 2
8.  **[Ground -> Sat 2]** ยิงคำสั่งวิทยุสั่งให้ดวงที่ 2 ไปเริ่มเก็บต่อ `RESUME:13`
9.  **[Sat 2]** รับช่วงต่อจาก ID 13 และวนลูป 10 วินาที ส่งแพ็คเกจไบนารีลงมาแบบสดๆ เหมือนเดิม
10. **[Sat 2 -> Ground]** หมด 10 วิ ส่งข้อความ `FINAL` ปิดจบ
11. **[Ground]** แสดงสรุปผลหน้าจอ พร้อมรับคำสั่งรอบใหม่

---

## 3. ตัวอย่าง Log บนจอ (Real-time Downlink)

**Ground Station:**
```text
[GROUND] Constellation Control Ready!
Type 'start' to simulate Sat 1 flying over.

[CMD] Sending START_PASS to Sat 1...
[TELEMETRY] ID: 0   | Time: 5432 ms | Batt: 3.84V | Temp: 22.1C | RSSI: -42 dBm
[TELEMETRY] ID: 1   | Time: 6210 ms | Batt: 3.91V | Temp: 24.5C | RSSI: -45 dBm
[TELEMETRY] ID: 2   | Time: 6980 ms | Batt: 4.10V | Temp: 31.0C | RSSI: -43 dBm
...
[GROUND] Sat 1 Pass Complete. Last ID successfully received: 12
[GROUND] Instructing Sat 2 to resume from ID: 13 in 3 seconds...

[TELEMETRY] ID: 13  | Time: 20100 ms| Batt: 3.75V | Temp: 28.4C | RSSI: -50 dBm
>> [WARNING] PACKET LOSS DETECTED! Expected 14, but got 15 <<
[TELEMETRY] ID: 15  | Time: 21500 ms| Batt: 4.05V | Temp: 21.9C | RSSI: -48 dBm
...

====================================
[GROUND] Constellation Mission Complete!
Total packets collected and downlinked up to ID: 25
====================================
```
