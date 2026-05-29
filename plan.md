  Roadmap ถ้าอยากไปให้สุดทาง (Fine-Tuning & Data Science)
  ถ้าคุณอยากทำโปรเจกต์ระดับจบมหาลัย หรือทำพอร์ตไปสมัครงาน Data Scientist สาย Edge Computing นี่คือ "ท่ามาตรฐาน" ที่คุณต้องทำครับ:

   1. Data Collection (ฝั่ง Laptop):
      * เก็บข้อมูล Data  เช่น สมุดประวัติคนไข้, คู่มือซ่อมรถของบริษัท, หรือแชทส่วนตัวของคุณ
   2. Fine-Tuning (ทำบน Cloud หรือ Laptop):
      * เอาข้อมูลไป Fine-tune ใส่โมเดล (เช่น Llama 3) โดยใช้เทคนิค LoRA / QLoRA บนคอมพิวเตอร์ที่มีการ์ดจอ หรือใช้ของฟรีอย่าง Google Colab (T4 GPU)
   3. Model Export & Quantization :
      * โมเดลที่เทรนเสร็จจะมีขนาดใหญ่มาก (เช่น 16GB) ต้องใช้เครื่องมือ (เช่น llama.cpp) แปลงไฟล์ให้เป็นนามสกุล .gguf และลดทอนความละเอียด (Quantize) จาก 16-bit
        เป็น 4-bit เพื่อให้ขนาดเล็กลงเหลือ 1-2GB
   4. Deployment (บน Raspberry Pi):
      * โยนไฟล์ .gguf ที่คุณเทรนและบีบอัดเองเสร็จแล้ว เข้าไปใน Raspberry Pi
      * ใช้รันผ่าน Ollama หรือ llama.cpp เป็น API Server
      * เขียนเว็บหรือแอปมือถือ ยิง API มาที่ Raspberry Pi เพื่อดึงคำตอบจาก "โมเดลเฉพาะทางที่สร้างขึ้นเอง"