import time
import threading
import random
import os

class LoraNetwork:
    def __init__(self):
        self.channel = []

    def send(self, msg):
        self.channel.append(msg)

    def receive(self):
        if len(self.channel) > 0:
            return self.channel.pop(0)
        return None

    def has_messages(self):
        return len(self.channel) > 0


class OrbitSimulator:
    def __init__(self):
        self.orbit_path = [
            ("Pacific Ocean", 10),
            ("Japan", 5),
            ("China", 8),
            ("Thailand", 7),
            ("Indian Ocean", 10),
            ("Africa", 8)
        ]
        self.current_index = 0
        self.current_location = self.orbit_path[0][0]
        self.is_over_thailand = False

    def run(self):
        print("[Orbit Sim] Satellite launched into orbit...")
        while True:
            loc_name, duration = self.orbit_path[self.current_index]
            self.current_location = loc_name
            self.is_over_thailand = (loc_name == "Thailand")
            
            print(f"\n🌍 [Orbit] Satellite is now passing over: ** {loc_name.upper()} **")
            if self.is_over_thailand:
                print("   -> 🇹🇭 In range of Thailand Ground Station! Link Established.")
            else:
                print("   -> 🔇 Out of range of Thailand. Storing data...")
                
            time.sleep(duration)
            self.current_index = (self.current_index + 1) % len(self.orbit_path)


class CubeSat:
    def __init__(self, network, orbit):
        self.network = network
        self.orbit = orbit
        self.data_file = "simulated_sd_card.txt"
        self._initialize_sd_card()

    def _initialize_sd_card(self):
        with open(self.data_file, "w") as f:
            f.write("")

    def run(self):
        print("[CubeSat 2] Booting up... Store and Forward system active.")
        last_log_time = time.time()
        
        while True:
            current_time = time.time()
            
            if current_time - last_log_time >= 2.0:
                self._store_data(current_time)
                last_log_time = current_time
                
            self._check_commands()
            time.sleep(0.1)

    def _store_data(self, current_time):
        temp = round(25.0 + random.uniform(-1.0, 1.0), 2)
        volt = round(3.7 + random.uniform(-0.2, 0.2), 2)
        data_str = f"Loc:{self.orbit.current_location},{int(current_time)},{temp}C,{volt}V"
        
        with open(self.data_file, "a") as f:
            f.write(data_str + "\n")
            
        if not self.orbit.is_over_thailand:
             print(f"[CubeSat 2] 💾 Stored data: {data_str}")

    def _check_commands(self):
        if self.network.has_messages():
            msg = self.network.channel[0]  # Peek
            if msg.startswith("GND_TO_SAT"):
                self.network.receive() # Consume
                command = msg.split(":")[1]
                
                if command == "SAT2_DUMP":
                    self._handle_dump_command()

    def _handle_dump_command(self):
        if not self.orbit.is_over_thailand:
            print("[CubeSat 2] ❌ Error: Received command but not over Thailand. Ignored.")
            return
            
        print("\n[CubeSat 2] 📡 Received Command: DUMP. Starting transmission...")
        try:
            with open(self.data_file, "r") as f:
                lines = f.readlines()
                if not lines:
                    print("[CubeSat 2] No data to send.")
                for line in lines:
                    tx_msg = f"SAT_TO_GND:{line.strip()}"
                    self.network.send(tx_msg)
                    print(f"[CubeSat 2] 📤 Transmitting: {tx_msg}")
                    time.sleep(0.3)
            print("[CubeSat 2] ✅ Transmission Complete. Clearing SD Card...")
            self._initialize_sd_card()
        except FileNotFoundError:
            print("[CubeSat 2] ❌ SD Card File not found!")


class GroundStation:
    def __init__(self, network, orbit):
        self.network = network
        self.orbit = orbit

    def run(self):
        print("[Thailand Station] Active. Waiting for satellite...")
        print("Type 'dump' when satellite is overhead to request data, or 'exit' to quit.")
        
        while True:
            cmd = input()
            if cmd.lower() == 'dump':
                self._request_dump()
            elif cmd.lower() == 'exit':
                print("Exiting simulation...")
                os._exit(0)

    def _request_dump(self):
        if not self.orbit.is_over_thailand:
            print("[Thailand Station] ⚠️ Satellite is NOT in range right now! Command will fail.")
            
        print("\n[Thailand Station] 🚀 Sending command to CubeSat 2...")
        self.network.send("GND_TO_SAT:SAT2_DUMP")
        
        print("[Thailand Station] 🎧 Listening for data...")
        timeout = 5
        while timeout > 0:
            msg = self.network.receive()
            if msg and msg.startswith("SAT_TO_GND"):
                data = msg.split(":")[1]
                print(f"[Thailand Station] 📥 Received: {data}")
                timeout = 5 
            else:
                if msg: # Put it back if it's not ours
                    self.network.send(msg)
                time.sleep(1)
                timeout -= 1
        print("[Thailand Station] 🛑 Reception ended.")


if __name__ == "__main__":
    network = LoraNetwork()
    orbit = OrbitSimulator()
    cubesat = CubeSat(network, orbit)
    ground_station = GroundStation(network, orbit)
    
    orbit_thread = threading.Thread(target=orbit.run, daemon=True)
    orbit_thread.start()
    
    sat_thread = threading.Thread(target=cubesat.run, daemon=True)
    sat_thread.start()
    
    time.sleep(1) 
    ground_station.run()
