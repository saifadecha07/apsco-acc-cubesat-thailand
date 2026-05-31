import time
import threading
import random

# --- Simulated SD Card (File on PC) ---
DATA_FILE = "simulated_sd_card.txt"

# --- Simulated LoRa Network (Message Queue) ---
lora_channel = []

# --- Orbit Simulation ---
# List of regions the CubeSat flies over, and duration in seconds
ORBIT_PATH = [
    ("Pacific Ocean", 10),
    ("Japan", 5),
    ("China", 8),
    ("Thailand", 7), # Only 7 seconds over Thailand!
    ("Indian Ocean", 10),
    ("Africa", 8)
]
current_location_index = 0
current_location = ORBIT_PATH[0][0]
is_over_thailand = False

def orbit_task():
    global current_location_index, current_location, is_over_thailand
    print("[Orbit Sim] Satellite launched into orbit...")
    while True:
        loc_name, duration = ORBIT_PATH[current_location_index]
        current_location = loc_name
        is_over_thailand = (loc_name == "Thailand")
        
        print(f"\n🌍 [Orbit] Satellite is now passing over: ** {loc_name.upper()} **")
        if is_over_thailand:
            print("   -> 🇹🇭 In range of Thailand Ground Station! Link Established.")
        else:
            print("   -> 🔇 Out of range of Thailand. Storing data...")
            
        time.sleep(duration)
        
        # Move to next location in orbit
        current_location_index = (current_location_index + 1) % len(ORBIT_PATH)

def cubesat_task():
    print("[CubeSat 2] Booting up... Store and Forward system active.")
    with open(DATA_FILE, "w") as f:
        f.write("")
        
    last_log_time = time.time()
    
    while True:
        current_time = time.time()
        
        # 1. Store Phase: Log data every 2 seconds to build up data quickly
        if current_time - last_log_time >= 2.0:
            temp = round(25.0 + random.uniform(-1.0, 1.0), 2)
            volt = round(3.7 + random.uniform(-0.2, 0.2), 2)
            data_str = f"Loc:{current_location},{int(current_time)},{temp}C,{volt}V"
            
            with open(DATA_FILE, "a") as f:
                f.write(data_str + "\n")
            # Only print storing message if NOT dumping to keep console clean
            if not is_over_thailand:
                 print(f"[CubeSat 2] 💾 Stored data: {data_str}")
            last_log_time = current_time
            
        # 2. Forward Phase: Check for commands
        if len(lora_channel) > 0 and lora_channel[0].startswith("GND_TO_SAT"):
            msg = lora_channel.pop(0)
            command = msg.split(":")[1]
            
            if command == "SAT2_DUMP":
                if not is_over_thailand:
                    print("[CubeSat 2] ❌ Error: Received command but not over Thailand. Ignored.")
                    continue
                    
                print("\n[CubeSat 2] 📡 Received Command: DUMP. Starting transmission...")
                try:
                    with open(DATA_FILE, "r") as f:
                        lines = f.readlines()
                        if not lines:
                            print("[CubeSat 2] No data to send.")
                        for line in lines:
                            tx_msg = f"SAT_TO_GND:{line.strip()}"
                            lora_channel.append(tx_msg)
                            print(f"[CubeSat 2] 📤 Transmitting: {tx_msg}")
                            time.sleep(0.3) # Simulate transmission delay
                    print("[CubeSat 2] ✅ Transmission Complete. Clearing SD Card...")
                    with open(DATA_FILE, "w") as f: # Clear file after success
                        f.write("")
                except FileNotFoundError:
                    print("[CubeSat 2] ❌ SD Card File not found!")
                    
        time.sleep(0.1)

def ground_station_task():
    print("[Thailand Station] Active. Waiting for satellite...")
    print("Type 'dump' when satellite is overhead to request data, or 'exit' to quit.")
    
    while True:
        cmd = input()
        if cmd.lower() == 'dump':
            if not is_over_thailand:
                print("[Thailand Station] ⚠️ Satellite is NOT in range right now! Command will fail.")
                
            print("\n[Thailand Station] 🚀 Sending command to CubeSat 2...")
            lora_channel.append("GND_TO_SAT:SAT2_DUMP")
            
            print("[Thailand Station] 🎧 Listening for data...")
            timeout = 5
            while timeout > 0:
                if len(lora_channel) > 0 and lora_channel[0].startswith("SAT_TO_GND"):
                    msg = lora_channel.pop(0)
                    data = msg.split(":")[1]
                    print(f"[Thailand Station] 📥 Received: {data}")
                    timeout = 5 
                else:
                    time.sleep(1)
                    timeout -= 1
            print("[Thailand Station] 🛑 Reception ended.")
            
        elif cmd.lower() == 'exit':
            print("Exiting simulation...")
            import os
            os._exit(0)

if __name__ == "__main__":
    # Start Orbit Tracker
    orbit_thread = threading.Thread(target=orbit_task, daemon=True)
    orbit_thread.start()
    
    # Start CubeSat logic
    sat_thread = threading.Thread(target=cubesat_task, daemon=True)
    sat_thread.start()
    
    time.sleep(1) 
    ground_station_task()
