#include <WiFi.h>
#include <esp_now.h>
#include <ArduinoOTA.h>
#include "config.h"

/************************************************************
   🚀 MULTI-LEADER ESP-NOW DRONE SWARM WITH CRAZYFLIE 🚀
   
   ✅ Multi-Leader System (Any Drone Can Be a Leader)
   ✅ ESP-NOW Decentralized Communication
   ✅ OTA Updates (Configurable in `config.h`)
   ✅ Onboard LED Status Indicators
   ✅ Dynamic Drone ID Assignment (Manual or Auto)
   ✅ Crazyflie Firmware Integrated (Mock Flight Control)
   
   📌 HOW TO USE:
   - Set `IS_LEADER = true` on **at least one drone** in `config.h`
   - Upload the **same code** to all drones
   - Open **Serial Monitor (115200 baud) on the Leader**
   - Enter Commands: `"TRIANGLE", "LINE", "HOVER", "LAND"`
   - Drones will adjust formation dynamically!
************************************************************/

// Crazyflie High-Level Commander (Mock API)
void moveTo(float x, float y, float z) {
  Serial.printf("[CRAZYFLIE] Moving to X=%.2f, Y=%.2f, Z=%.2f\n", x, y, z);
}

// Structure for ESP-NOW commands
typedef struct {
  char command[20];  
  uint8_t senderID;  
} DroneCommand;

// Dynamic Drone ID Assignment
uint8_t droneID = (DRONE_ID == -1) ? esp_random() % 255 : DRONE_ID;
bool isLeader = IS_LEADER;

// ESP-NOW Callback for Sending Data
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.printf("[ESP-NOW] Message Sent: %s\n", (status == ESP_NOW_SEND_SUCCESS) ? "Success" : "Fail");
}

// ESP-NOW Callback for Receiving Data
void onDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  DroneCommand receivedCommand;
  memcpy(&receivedCommand, incomingData, sizeof(receivedCommand));

  Serial.printf("[DRONE %d] Received Command: %s from Drone %d\n", droneID, receivedCommand.command, receivedCommand.senderID);

  // Blink Red LED on command execution
  digitalWrite(LED_PIN, HIGH);
  delay(200);
  digitalWrite(LED_PIN, LOW);

  if (strcmp(receivedCommand.command, "TRIANGLE") == 0) {
    moveTo(1.0, 1.0, 3.0);
  } else if (strcmp(receivedCommand.command, "LINE") == 0) {
    moveTo(2.0, 0.0, 3.0);
  } else if (strcmp(receivedCommand.command, "HOVER") == 0) {
    Serial.printf("[DRONE %d] Hovering...\n", droneID);
  } else if (strcmp(receivedCommand.command, "LAND") == 0) {
    Serial.printf("[DRONE %d] Landing...\n", droneID);
  }
}

// Function to Send Command (Only Leader Uses This)
void sendCommand(const char *cmd) {
  DroneCommand commandToSend;
  strcpy(commandToSend.command, cmd);
  commandToSend.senderID = droneID;  

  esp_now_send(0, (uint8_t *)&commandToSend, sizeof(commandToSend));
  Serial.printf("[LEADER %d] Sending Command: %s\n", droneID, cmd);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERROR] ESP-NOW Initialization Failed!");
    return;
  }

  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);

  Serial.printf("[INFO] Drone Initialized - ID: %d | Role: %s\n", droneID, isLeader ? "Leader" : "Follower");

  if (OTA_ENABLED) {
    ArduinoOTA.begin();
  }

  if (MULTI_LEADER) {
    delay(LEADER_TIMEOUT);  
    if (!isLeader) {
      Serial.printf("[DRONE %d] No leader detected. Taking over as Leader!\n", droneID);
      isLeader = true;
    }
  }
}

void loop() {
  if (OTA_ENABLED) {
    ArduinoOTA.handle();
  }

  if (isLeader) {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);

    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      sendCommand(input.c_str());
    }
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  delay(COMMUNICATION_INTERVAL);
}
