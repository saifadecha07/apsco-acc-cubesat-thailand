# LoRa Constellation Store-and-Forward Workflow

เอกสารนี้อธิบายลำดับการทำงานของระบบ "Constellation Simulator" ระดับอุตสาหกรรม ซึ่งจำลองสถาปัตยกรรม **"Store-and-Forward" (Data Dump)** ที่ใช้ในดาวเทียม CubeSat ที่อยู่นอกพื้นที่รับสัญญาณ ก่อนที่จะเทข้อมูลทั้งหมด (Data Dump) ลงมายังสถานีภาคพื้นเมื่อบินผ่าน

---

## 1. การจัดโครงสร้างข้อมูล (Telemetry Data Structure)
ข้อมูลถูกแพ็คเป็น **C++ Binary Struct** ขนาดเล็กและบันทึกลงในหน่วยความจำ (Buffer/SD Card) ก่อนส่ง:

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
3.  **[Sat 1] STORE PHASE:** รับคำสั่ง แล้ววนลูป 10 วินาที เพื่อเก็บข้อมูลเงียบๆ (Store) ลงใน Array
    *   สร้าง `TelemetryPacket` (สุ่มอุณหภูมิ/แบตเตอรี่)
    *   เก็บบันทึกลงตัวแปร `buffer` (จำลอง SD Card)
    *   *ในช่วงนี้ ฝั่ง Ground จะยังไม่เห็นข้อมูลใดๆ*
4.  **[Sat 1 -> Ground] FORWARD PHASE:** เมื่อครบ 10 วิ ดาวเทียมจะทำการ **Data Dump** สาดแพ็คเกจไบนารีทั้งหมดใน Array ลงมายัง Ground อย่างรวดเร็วทีละก้อนต่อเนื่องกัน
5.  **[Ground]** ตรวจจับ Data Dump Burst ได้ และแสดงผลแพ็คเกจที่ทะลักเข้ามาอย่างรวดเร็ว
    *   *ระบบป้องกัน:* ตรวจสอบ `packet_id` ป้องกันการสูญหายระหว่าง Burst Transmission
6.  **[Sat 1 -> Ground]** เมื่อสาดข้อมูลหมดแล้ว ส่งวิทยุข้อความ `HANDOVER`
7.  **[Ground]** รับคำสั่ง `HANDOVER` ดูว่าตัวเองได้รับ ID ล่าสุดถึงเลขอะไร แล้วบวก 1 เป็นเลขถัดไป
8.  **[Ground]** หน่วงเวลา 3 วินาที
9.  **[Ground -> Sat 2]** ยิงคำสั่ง `RESUME:X` ไปให้ดาวเทียมดวงที่ 2 เริ่มทำงานต่อ
10. **[Sat 2]** รับช่วงต่อจาก ID ถัดไป เข้าสู่ STORE PHASE เก็บข้อมูลเงียบๆ 10 วิ และตามด้วย FORWARD PHASE เพื่อทำ Data Dump
11. **[Sat 2 -> Ground]** เมื่อสาดข้อมูลหมด ส่งข้อความ `FINAL` ปิดจบ
12. **[Ground]** แสดงสรุปผลหน้าจอ

---

## 3. ตัวอย่าง Log บนจอ

**Ground Station:**
```text
[GROUND] Constellation Control Ready!
Type 'start' to simulate Sat 1 flying over.

[CMD] Sending START_PASS to Sat 1...
[GROUND] Sat 1 is now in STORE phase (collecting silently for 10s)...

[DETECT] Incoming DATA DUMP burst detected! Receiving...
  -> [DUMP] ID: 0   | Time: 5432 ms | Batt: 3.84V | Temp: 22.1C | RSSI: -42 dBm
  -> [DUMP] ID: 1   | Time: 6210 ms | Batt: 3.91V | Temp: 24.5C | RSSI: -45 dBm
  -> [DUMP] ID: 2   | Time: 6980 ms | Batt: 4.10V | Temp: 31.0C | RSSI: -43 dBm
...
[GROUND] Data Dump Complete! Last ID successfully received: 14
[GROUND] Instructing Sat 2 to resume from ID: 15 in 3 seconds...

[CMD] Sending RESUME to Sat 2...
[GROUND] Sat 2 is now in STORE phase (collecting silently for 10s)...

[DETECT] Incoming DATA DUMP burst detected! Receiving...
  -> [DUMP] ID: 15  | Time: 20100 ms| Batt: 3.75V | Temp: 28.4C | RSSI: -50 dBm
>> [WARNING] PACKET LOSS DETECTED! Expected 16, but got 17 <<
  -> [DUMP] ID: 17  | Time: 21500 ms| Batt: 4.05V | Temp: 21.9C | RSSI: -48 dBm
...

====================================
[GROUND] Constellation Mission Complete!
Total packets collected via Data Dumps up to ID: 28
====================================
```
