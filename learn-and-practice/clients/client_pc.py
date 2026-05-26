import socket

# IP ของ Raspberry Pi
HOST = '192.168.137.56' 
PORT = 65432

# ข้อความที่ต้องการจะส่ง
msg_to_send = "สวัสดี Raspberry Pi จาก Laptop!"

print(f"กำลังเชื่อมต่อไปหา {HOST}...")

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    try:
        s.connect((HOST, PORT))
        print("เชื่อมต่อสำเร็จ กำลังส่งข้อความ...")
        
        # ส่งข้อความไป
        s.sendall(msg_to_send.encode('utf-8'))
        
        # รอรับข้อความตอบกลับจาก Pi
        data = s.recv(1024)
        print(f"ข้อความตอบกลับจาก Pi: {data.decode('utf-8')}")
    except ConnectionRefusedError:
         print("เชื่อมต่อไม่สำเร็จ เช็ก IP หรือดูว่า Pi รันสคริปต์อยู่หรือเปล่า")