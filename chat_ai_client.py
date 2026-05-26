import socket

HOST = '192.168.137.56' 
PORT = 65432

print("====================================")
print("  Raspberry Pi AI Chat Controller   ")
print("====================================")
print("กำลังเชื่อมต่อไปหา AI บน Raspberry Pi...")

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    try:
        s.connect((HOST, PORT))
        print("เชื่อมต่อสำเร็จ! พิมพ์คุยได้เลย (พิมพ์ 'exit' เพื่อออก)")
        print("ลองพิมพ์: 'how hot are you', 'check memory', หรือ 'hello'\n")
        
        while True:
            msg = input("คุณ: ")
            if msg.lower() == 'exit':
                break
            if msg.strip() == '':
                continue
                
            # ส่งแชทไปหา Pi
            s.sendall(msg.encode('utf-8'))
            
            # รอ Pi (AI) ตอบกลับ
            data = s.recv(4096)
            print(f"Pi AI: {data.decode('utf-8')}\n")
            
    except ConnectionRefusedError:
         print("เชื่อมต่อไม่สำเร็จ! เช็กว่า Pi รันคำสั่ง python3 ai_server.py หรือยัง")