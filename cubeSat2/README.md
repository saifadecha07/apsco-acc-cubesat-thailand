# CubeSat 2: Store-and-Forward Payload

เอกสารนี้อธิบายการทำงานของระบบ Payload สำหรับ CubeSat 2 ซึ่งออกแบบมาในลักษณะ **Store-and-Forward** (เก็บข้อมูลไว้ก่อน แล้วค่อยส่งทีเดียวเมื่อได้รับการร้องขอ)

## 📁 ไฟล์ในโปรเจกต์
- `cubesat2_payload.ino` - โค้ดหลักสำหรับไมโครคอนโทรลเลอร์ (Arduino/ESP32)

## 🛰️ หลักการทำงาน (Store-and-Forward)
ระบบแบ่งการทำงานออกเป็น 2 ส่วนหลัก:

1. **Store Phase (เก็บข้อมูล):** 
   - ดาวเทียมจะทำงานในอวกาศและอ่านค่าจากเซนเซอร์ (เช่น อุณหภูมิ, แรงดันแบตเตอรี่, สถานะระบบ) ตามช่วงเวลาที่กำหนด (เช่น ทุกๆ 5 วินาที)
   - ข้อมูลจะถูกบันทึกลงใน **SD Card** เป็นไฟล์ Text (เช่น `data.txt`) ในรูปแบบ CSV (Comma-Separated Values) พร้อมกับ Timestamp

2. **Forward Phase (ส่งข้อมูลลงสถานีภาคพื้นดิน):**
   - โมดูลรับส่งสัญญาณวิทยุ (LoRa) จะคอยฟังคำสั่ง (Listen) อย่างต่อเนื่อง
   - เมื่อสถานีภาคพื้นดิน (Ground Station) หรือดาวเทียมดวงอื่นส่งคำสั่งรหัส **`SAT2:DUMP`** ขึ้นมา
   - CubeSat 2 จะเปิดไฟล์ใน SD Card แล้วส่งข้อมูลทั้งหมดที่เก็บไว้ออกมาทาง LoRa ทีละบรรทัดจนจบ

## 📡 โครงสร้างการเชื่อมต่อ (Network Topology สำหรับ 2 CubeSats)

สามารถนำไปประยุกต์ใช้กับระบบที่มีดาวเทียม 2 ดวงได้ 2 รูปแบบ:

*   **แบบที่ 1: ติดต่อตรงกับ Ground Station (Direct Link)**
    *   Ground Station ส่ง `SAT1:DUMP` -> CubeSat 1 ส่งข้อมูลลงมา
    *   Ground Station ส่ง `SAT2:DUMP` -> CubeSat 2 ส่งข้อมูลลงมา
*   **แบบที่ 2: ดาวเทียมทำหน้าที่เป็น Relay (Inter-Satellite Link - ISL)**
    *   หาก CubeSat 2 อยู่ห่างไกลกว่า Ground Station จะส่งคำสั่งไปที่ CubeSat 1
    *   CubeSat 1 ทำหน้าที่เป็น Relay ทวนสัญญาณคำสั่ง `SAT2:DUMP` ไปให้ CubeSat 2
    *   CubeSat 2 ส่งข้อมูลกลับมาที่ CubeSat 1 แล้ว CubeSat 1 ส่งต่อลง Ground Station

## 🛠️ ฮาร์ดแวร์ที่ต้องใช้ (Hardware Requirements)
1. **Microcontroller:** Arduino (Uno, Nano, Mega) หรือ ESP32 / ESP8266
2. **LoRa Module:** เช่น SX1278 (433MHz) หรือ RFM95 (868/915MHz)
3. **SD Card Module:** สำหรับบันทึกข้อมูล (ใช้การเชื่อมต่อแบบ SPI)

## 📌 การตั้งค่าพิน (Pin Configuration) - ค่าเริ่มต้น
*สามารถปรับเปลี่ยนได้ในโค้ดบรรทัดที่ 7-12*
- **LoRa CS (NSS):** Pin 10
- **LoRa Reset:** Pin 9
- **LoRa IRQ (DIO0):** Pin 2
- **SD Card CS:** Pin 4
