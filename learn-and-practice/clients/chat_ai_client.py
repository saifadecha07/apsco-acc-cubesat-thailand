import socket

class AIChatClient:
    def __init__(self, host='192.168.137.56', port=65432):
        self.host = host
        self.port = port

    def start(self):
        print("====================================")
        print("  Raspberry Pi AI Chat Controller   ")
        print("====================================")
        print("กำลังเชื่อมต่อไปหา AI บน Raspberry Pi...")

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            try:
                s.connect((self.host, self.port))
                print("เชื่อมต่อสำเร็จ! พิมพ์คุยได้เลย (พิมพ์ 'exit' เพื่อออก)")
                print("ลองพิมพ์: 'how hot are you', 'check memory', หรือ 'hello'\n")
                
                self._chat_loop(s)
                    
            except ConnectionRefusedError:
                print("เชื่อมต่อไม่สำเร็จ! เช็กว่า Pi รันคำสั่ง python3 ai_server.py หรือยัง")

    def _chat_loop(self, connection):
        while True:
            msg = input("คุณ: ")
            if msg.lower() == 'exit':
                break
            if msg.strip() == '':
                continue
                
            connection.sendall(msg.encode('utf-8'))
            
            data = connection.recv(4096)
            print(f"Pi AI: {data.decode('utf-8')}\n")

if __name__ == "__main__":
    client = AIChatClient()
    client.start()
