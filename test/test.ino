#include <WiFi.h>
#include <WebServer.h>

const char *ssid = "ESP32_MagicTouch";
const char *password = "12345678";

WebServer server(80);

// โค้ดหน้าเว็บ HTML แสดงเกจวัดการสัมผัส 3 จุดพร้อมกัน
const char* html_page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Magic Touch</title>
  <style>
    body { font-family: 'Segoe UI', Arial, sans-serif; background-color: #0d1117; color: #c9d1d9; text-align: center; margin: 0; padding: 20px; }
    h1 { color: #58a6ff; font-size: 2rem; margin-bottom: 20px; }
    .card-container { display: flex; flex-direction: column; gap: 15px; align-items: center; }
    .card { background-color: #161b22; border: 1px solid #30363d; border-radius: 10px; padding: 20px; width: 90%; max-width: 400px; box-shadow: 0 4px 6px rgba(0,0,0,0.3); }
    .value-text { font-size: 2.5rem; font-weight: bold; color: #3fb950; margin: 10px 0; transition: color 0.2s; }
    .label { font-size: 1.2rem; color: #8b949e; font-weight: bold; }
    .desc { font-size: 0.8rem; color: #666; margin-top: 5px; }
    
    .progress-bg { background-color: #21262d; border-radius: 10px; width: 100%; height: 25px; margin-top: 15px; overflow: hidden; position: relative; }
    .progress-bar { height: 100%; background-color: #3fb950; width: 0%; transition: width 0.1s ease-out, background-color 0.2s; }
    
    .status-badge { display: inline-block; padding: 5px 15px; border-radius: 20px; font-size: 0.9rem; font-weight: bold; margin-top: 10px; background-color: #21262d; border: 1px solid #30363d; }
    .touched { background-color: rgba(248, 81, 73, 0.2); border-color: #f85149; color: #f85149; box-shadow: 0 0 10px rgba(248, 81, 73, 0.5); }
  </style>
</head>
<body>
  <h1>ESP32 Magic Touch Dashboard</h1>
  <p style="color: #8b949e; margin-bottom: 30px;">เอานิ้วแตะที่ขาเหล็กของบอร์ดเพื่อดูปฏิกิริยาแบบ Real-time</p>
  
  <div class="card-container">
    <!-- Touch 1 -->
    <div class="card">
      <div class="label">Touch Sensor 1 (Pin 4)</div>
      <div class="value-text" id="val-t4">0</div>
      <div class="progress-bg"><div class="progress-bar" id="bar-t4"></div></div>
      <div class="status-badge" id="badge-t4">STANDBY</div>
    </div>

    <!-- Touch 2 -->
    <div class="card">
      <div class="label">Touch Sensor 2 (Pin 13)</div>
      <div class="value-text" id="val-t13">0</div>
      <div class="progress-bg"><div class="progress-bar" id="bar-t13"></div></div>
      <div class="status-badge" id="badge-t13">STANDBY</div>
    </div>

    <!-- Touch 3 -->
    <div class="card">
      <div class="label">Touch Sensor 3 (Pin 14)</div>
      <div class="value-text" id="val-t14">0</div>
      <div class="progress-bg"><div class="progress-bar" id="bar-t14"></div></div>
      <div class="status-badge" id="badge-t14">STANDBY</div>
    </div>
  </div>

  <script>
    const TOUCH_THRESHOLD = 30; // ค่าถ้าน้อยกว่านี้ถือว่าแตะโดน (อาจต้องปรับตามสภาพบอร์ด)

    function updateCard(pin, value) {
      let valElem = document.getElementById("val-t" + pin);
      let barElem = document.getElementById("bar-t" + pin);
      let badgeElem = document.getElementById("badge-t" + pin);

      valElem.innerText = value;
      
      // แปลงค่าให้เป็น % (ปกติแตะ = ค่าน้อย, ปล่อย = ค่ามาก)
      let percent = (100 - value);
      if(percent < 0) percent = 0;
      if(percent > 100) percent = 100;
      barElem.style.width = percent + "%";

      if(value < TOUCH_THRESHOLD) {
        barElem.style.backgroundColor = "#f85149";
        valElem.style.color = "#f85149";
        badgeElem.innerText = "DETECTED!";
        badgeElem.classList.add("touched");
      } else {
        barElem.style.backgroundColor = "#3fb950";
        valElem.style.color = "#3fb950";
        badgeElem.innerText = "STANDBY";
        badgeElem.classList.remove("touched");
      }
    }

    function fetchData() {
      fetch('/data')
        .then(response => response.json())
        .then(data => {
          updateCard("4", data.t4);
          updateCard("13", data.t13);
          updateCard("14", data.t14);
        })
        .catch(err => console.log(err));
    }

    // ดึงข้อมูลรัวๆ ทุกๆ 0.15 วินาที
    setInterval(fetchData, 150);
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", html_page);
}

void handleData() {
  // อ่านค่าจากเซ็นเซอร์สัมผัส 3 จุดพร้อมกัน
  int t4 = touchRead(4);   // ขา Pin 4
  int t13 = touchRead(13); // ขา Pin 13
  int t14 = touchRead(14); // ขา Pin 14

  // สร้าง JSON
  String json = "{";
  json += "\"t4\":" + String(t4) + ",";
  json += "\"t13\":" + String(t13) + ",";
  json += "\"t14\":" + String(t14);
  json += "}";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(1000); 

  Serial.println("\n--------------------------");
  Serial.println("Starting Multi-Touch AP...");
  
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  
  Serial.print("SSID: "); Serial.println(ssid);
  Serial.print("IP Address: "); Serial.println(IP);

  server.on("/", handleRoot);
  server.on("/data", handleData);

  server.begin();
  Serial.println("Dashboard is live! Waiting for connection...");
}

void loop() {
  server.handleClient();
}
