import socket

class PiClient:
    def __init__(self, host='192.168.137.56', port=65432):
        self.host = host
        self.port = port

    def send_message(self, msg_to_send):
        print(f"กำลังเชื่อมต่อไปหา {self.host}...")
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            try:
                s.connect((self.host, self.port))
                print("เชื่อมต่อสำเร็จ กำลังส่งข้อความ...")
                s.sendall(msg_to_send.encode('utf-8'))
                
                data = s.recv(1024)
                print(f"ข้อความตอบกลับจาก Pi: {data.decode('utf-8')}")
            except ConnectionRefusedError:
                print("เชื่อมต่อไม่สำเร็จ เช็ก IP หรือดูว่า Pi รันสคริปต์อยู่หรือเปล่า")

if __name__ == "__main__":
    client = PiClient()
    client.send_message("สวัสดี Raspberry Pi จาก Laptop!")
