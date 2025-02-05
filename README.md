# 🚀 ESP-NOW Multi-Leader Drone Swarm with Crazyflie & OTA

Welcome to the **ESP-NOW Multi-Leader Drone Swarm** project! This project enables **a decentralized swarm of drones** using **ESP-NOW communication** on **ESP32 boards**. The drones operate in a **multi-leader system**, where any drone can take leadership if needed. The system also integrates **Crazyflie firmware for flight control** (mocked for now) and **OTA updates for remote programming**.

---

## 📌 **Features**
✅ **Decentralized Swarm Communication** – No Wi-Fi needed, uses **ESP-NOW**.  
✅ **Multi-Leader Configuration** – Any drone can become a leader if needed.  
✅ **OTA (Over-the-Air) Updates** – Enable remote firmware updates.  
✅ **Dynamic Drone ID Assignment** – Drones get IDs dynamically or manually.  
✅ **Onboard LED Status Indicators** – Easily track drone states.  
✅ **Supports Up to 20 Drones** – More scalability options available.  
✅ **Real-Time Commands Execution** – Drones respond to formation & control commands.  

---

## 📡 **How ESP-NOW Communication Works**
🚀 **ESP-NOW is an ultra-fast wireless protocol** (like Bluetooth but faster).  
- **Leader drones send commands** (formations, hover, land).  
- **Followers listen and execute the commands** in real-time.  
- If no leader is found, a **new leader is elected automatically**.  

Each drone communicates **dynamically, without pre-configured connections!** 🎯

---

## 🛠️ **Hardware Requirements**
| Component | Quantity |
|-----------|----------|
| **ESP32 Dev Board** | 3+ (Leader + Followers) |
| **LiPo Battery** | 1 per drone |
| **Motors & Propellers** | If using for real flight |
| **Frame & Sensors** | If using for real flight |

---

## 📥 **Installation & Setup**
### 1️⃣ **Clone the Repository**
```
git clone https://github.com/skumar911/ESP32Drones.git
cd ESP32Drones
```

### 2️⃣ **Flash Firmware to ESP32**
Upload the **same code** (`drone.ino`) to all ESP32 drones.

#### **Configuration**
Modify `config.h` before flashing:
```
#define IS_LEADER true  // Set 'true' for Leader, 'false' for Followers
#define OTA_ENABLED false  // Enable OTA updates if needed
#define DRONE_ID -1  // Set a manual ID or keep '-1' for dynamic ID
```
Set **at least one drone as Leader** (`IS_LEADER = true`) before uploading.

---

## 🎨 **LED Status Indicators**
| **State** | **LED Behavior** | **Description** |
|-----------|----------------|----------------|
| **Leader Active** | 🔵 LED Blinks Every 1 sec | Leader Confirmation |
| **Follower Active** | 🟢 LED ON | Drone is a follower |
| **Executing Command** | 🔴 LED Flashes Twice | Movement Acknowledged |
| **OTA Update in Progress** | 🟠 LED Blinking Rapidly | OTA Update Running |

---

## 📜 **Available Commands**
| **Command** | **Action** |
|------------|----------|
| `TRIANGLE` | Drones form a **Triangle Formation**. |
| `LINE` | Drones align in a **Straight Line**. |
| `HOVER` | All drones **hover in place**. |
| `LAND` | All drones **land immediately**. |

🔹 **Note:** Commands are sent from the **Leader Drone** via **Serial Monitor**.

---

## 🌐 **OTA (Over-the-Air) Updates**
### **How to Enable OTA**
- **Set `OTA_ENABLED = true` in `config.h`**.
- Flash the updated firmware.
- Use the **Arduino IDE** or **ESPHome OTA** to upload new firmware wirelessly.

### **Performing an OTA Update**
1. Get the **IP Address** of the drone from the Serial Monitor.
2. Run the OTA upload:
   ```
   python espota.py -i <DRONE_IP> -p 3232 -f firmware.bin
   ```
   Example:
   ```
   python espota.py -i 192.168.1.101 -p 3232 -f drone_v2.bin
   ```

---

## 🛠️ **Debugging & Troubleshooting**
### 1️⃣ **No Response from Drones?**
✅ Ensure all drones are powered on.  
✅ Check if the Leader Drone is set correctly (`IS_LEADER = true`).  
✅ Use the Serial Monitor to **see if commands are being received**.  

### 2️⃣ **Leader Drone Not Recognized?**
- Set **one drone as a leader manually** in `config.h`.
- Ensure `LEADER_TIMEOUT` is long enough (`5000ms` or more).

### 3️⃣ **OTA Update Not Working?**
- Check that `OTA_ENABLED` is **set to `true`**.
- Ensure the **drone is connected to Wi-Fi** before performing an update.

---

## 🎯 **Future Improvements**
📌 **Scale to 50+ Drones** – Implement LoRa/Zigbee for larger swarms.  
📌 **Real Flight Control** – Enable full Crazyflie integration.  
📌 **Web-Based Control Interface** – Instead of Serial Monitor, use a website.  

---

Happy Coding! 🚀
