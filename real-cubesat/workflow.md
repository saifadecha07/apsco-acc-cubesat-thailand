# LoRa Constellation Handover Workflow

เอกสารนี้อธิบายลำดับการทำงานของระบบ "Constellation Simulator" ซึ่งจำลองสถานการณ์ที่ดาวเทียม 2 ดวง (Sat 1 และ Sat 2) บินผ่านสถานีภาคพื้นดินและทำงานสานต่อกัน โดยอ้างอิงการเก็บข้อมูลลง Array เริ่มจาก Index 0

---

## 1. บทบาทและอุปกรณ์

ระบบถูกแบ่งออกเป็น 2 โหนดหลัก แต่ทำงานจำลองเป็น 3 ส่วน:

### Node 1: Constellation Simulator (เครื่องของน้อง)
*   **ไฟล์โค้ด:** `real-cubesat/lora1_cubesat.ino`
*   **หน้าที่:** เป็นโปรแกรมจำลองที่สามารถสวมบทบาทเป็นได้ทั้ง Sat 1 และ Sat 2
    *   **บทบาท Sat 1:** เมื่อได้รับคำสั่งเริ่ม บินผ่านน่านฟ้า 10 วินาที เก็บข้อมูลโดยเริ่มจาก Array Index 0 แล้วส่งเลข Index สุดท้ายที่เก็บได้ (Handover) บอกสถานีภาคพื้น
    *   **บทบาท Sat 2:** เมื่อได้รับคำสั่งให้ทำงานต่อ จะเริ่มเก็บข้อมูลต่อจากเลข Array ถัดไปที่ Ground สั่ง บินอีก 10 วินาที แล้วส่งยอดเลข Index สุดท้าย (Final)

### Node 2: Ground Station (เครื่องของคุณ)
*   **ไฟล์โค้ด:** `real-cubesat/lora2_ground.ino`
*   **หน้าที่:** เป็นศูนย์ควบคุม
    *   สั่งการ Sat 1 ให้เริ่มเก็บข้อมูล
    *   รับข้อมูล Handover เพื่อดูเลข Index สุดท้าย แล้วคำนวณหมายเลข Array ถัดไป (+1)
    *   สั่งการให้ Sat 2 ไปทำงานต่อจากเลข Array ถัดไป

---

## 2. ลำดับการทำงานของระบบ (Sequence Diagram)

1.  **[Ground]** ผู้ใช้พิมพ์คำว่า `start`
2.  **[Ground -> Sat 1]** ส่งวิทยุสั่งการ `START_PASS`
3.  **[Sat 1]** ตอบรับคำสั่ง วนลูป 10 วินาทีเก็บข้อมูลเริ่มที่ 0 (สมมติความเร็วรอบนี้เก็บได้ถึง Index ที่ 49)
4.  **[Sat 1 -> Ground]** ส่งวิทยุบอก Ground ว่าทำเสร็จแล้วถึงเลขอะไร `HANDOVER:49`
5.  **[Ground]** รับทราบว่า Sat 1 เก็บข้อมูล Index 0-49 เรียบร้อยแล้ว
6.  **[Ground]** หน่วงเวลาจำลองรอ Sat 2 บินมาถึง
7.  **[Ground -> Sat 2]** ส่งวิทยุสั่ง Sat 2 ว่าให้ไปเก็บตัวที่ 50 เป็นต้นไป `RESUME:50`
8.  **[Sat 2]** ตอบรับคำสั่ง เริ่มเก็บต่อตั้งแต่เลข 50 วนลูปอีก 10 วินาที (สมมติเก็บได้ถึง Index ที่ 98)
9.  **[Sat 2 -> Ground]** ส่งเลข Index รวมสุดท้ายบอก Ground `FINAL:98`
10. **[Ground]** สรุปภารกิจสำเร็จ ขึ้นหน้าจอเตรียมพร้อมเริ่มรอบใหม่

---

## 3. ตัวอย่าง Log บนจอ

**Ground Station:**
```text
[GROUND] Constellation Control Ready!
Type 'start' to simulate Sat 1 flying over.

[CMD] Sending START_PASS to Sat 1...

[GROUND] Received Handover from Sat 1!
Sat 1 collected data index: 0 to 49
[GROUND] Calculating trajectory for Sat 2...
[GROUND] Instructing Sat 2 to resume from index 50...

====================================
[GROUND] Constellation Mission Complete!
Sat 2 finished collection up to Array index: 98
====================================
```

**Constellation Simulator (CubeSat Node):**
```text
[CONSTELLATION SIMULATOR] Ready!
Waiting for Ground commands...

[SAT 1] Entering coverage zone! Starting collection...
Collected Array Data #0
Collected Array Data #1
...
Collected Array Data #49
[SAT 1] Leaving coverage zone. Last index collected: 49
[SAT 1] Sending HANDOVER info to Ground...

[SAT 2] Entering coverage zone!
[SAT 2] Ground ordered to resume from Array #50
Collected Array Data #50
Collected Array Data #51
...
Collected Array Data #98
[SAT 2] Leaving coverage zone. Final Array index: 98
[SAT 2] Sending FINAL report to Ground...
```