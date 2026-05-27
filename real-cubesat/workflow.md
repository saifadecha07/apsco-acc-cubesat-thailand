# LoRa Constellation Handover Workflow

เอกสารนี้อธิบายลำดับการทำงานของระบบ "Constellation Simulator" ซึ่งจำลองสถานการณ์ที่ดาวเทียม 2 ดวง (Sat 1 และ Sat 2) บินผ่านสถานีภาคพื้นดินและทำงานสานต่อกัน

---

## 1. บทบาทและอุปกรณ์

ระบบถูกแบ่งออกเป็น 2 โหนดหลัก แต่ทำงานจำลองเป็น 3 ส่วน:

### Node 1: Constellation Simulator (เครื่องของน้อง)
*   **ไฟล์โค้ด:** `real-cubesat/lora1_cubesat.ino`
*   **หน้าที่:** เป็นโปรแกรมจำลองที่สามารถสวมบทบาทเป็นได้ทั้ง Sat 1 และ Sat 2
    *   **บทบาท Sat 1:** เมื่อได้รับคำสั่งเริ่ม บินผ่านน่านฟ้า 10 วินาที เก็บข้อมูลแบบสุ่มความเร็ว แล้วส่งยอดรวม (Handover) บอกสถานีภาคพื้น
    *   **บทบาท Sat 2:** เมื่อได้รับคำสั่งให้ทำงานต่อ จะเริ่มนับ Array ต่อจากเลขที่ Ground สั่ง บินอีก 10 วินาที แล้วส่งยอดสรุป (Final)

### Node 2: Ground Station (เครื่องของคุณ)
*   **ไฟล์โค้ด:** `real-cubesat/lora2_ground.ino`
*   **หน้าที่:** เป็นศูนย์ควบคุม
    *   สั่งการ Sat 1 ให้เริ่มเก็บข้อมูล
    *   รับข้อมูล Handover แล้วคำนวณหมายเลข Array ถัดไป (+1)
    *   สั่งการให้ Sat 2 ไปทำงานต่อจากเลขนั้น

---

## 2. ลำดับการทำงานของระบบ (Sequence Diagram)

1.  **[Ground]** ผู้ใช้พิมพ์คำว่า `start`
2.  **[Ground -> Sat 1]** ส่งวิทยุสั่งการ `START_PASS`
3.  **[Sat 1]** ตอบรับคำสั่ง วนลูป 10 วินาทีเก็บข้อมูล (สมมติความเร็วรอบนี้เก็บได้ 45 ตัว)
4.  **[Sat 1 -> Ground]** ส่งวิทยุบอก Ground ว่าทำเสร็จแล้ว `HANDOVER:45`
5.  **[Ground]** รับทราบว่า Sat 1 ได้ข้อมูล 1-45 
6.  **[Ground]** หน่วงเวลาจำลองรอ Sat 2 บินมาถึง
7.  **[Ground -> Sat 2]** ส่งวิทยุสั่ง Sat 2 ว่าให้ไปเก็บตัวที่ 46 เป็นต้นไป `RESUME:46`
8.  **[Sat 2]** ตอบรับคำสั่ง เริ่มเก็บต่อตั้งแต่เลข 46 วนลูปอีก 10 วินาที (สมมติเก็บได้ถึง 82)
9.  **[Sat 2 -> Ground]** ส่งยอดรวมสุดท้ายบอก Ground `FINAL:82`
10. **[Ground]** สรุปภารกิจสำเร็จ ขึ้นหน้าจอเตรียมพร้อมเริ่มรอบใหม่

---

## 3. ตัวอย่าง Log บนจอ

**Ground Station:**
```text
[GROUND] Constellation Control Ready!
Type 'start' to simulate Sat 1 flying over.

[CMD] Sending START_PASS to Sat 1...

[GROUND] Received Handover from Sat 1!
Sat 1 collected data index: 1 to 45
[GROUND] Calculating trajectory for Sat 2...
[GROUND] Instructing Sat 2 to resume from index 46...

====================================
[GROUND] Constellation Mission Complete!
Sat 2 finished collection up to Array index: 82
====================================
```

**Constellation Simulator (CubeSat Node):**
```text
[CONSTELLATION SIMULATOR] Ready!
Waiting for Ground commands...

[SAT 1] Entering coverage zone! Starting collection...
Collected Array Data #1
Collected Array Data #2
...
Collected Array Data #45
[SAT 1] Leaving coverage zone. Total collected: 45
[SAT 1] Sending HANDOVER info to Ground...

[SAT 2] Entering coverage zone!
[SAT 2] Ground ordered to resume from Array #46
Collected Array Data #46
Collected Array Data #47
...
Collected Array Data #82
[SAT 2] Leaving coverage zone. Final Array index: 82
[SAT 2] Sending FINAL report to Ground...
```