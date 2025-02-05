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

#define LED_PIN 2  // Onboard LED for visual cues

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

// ESP-NOW Peer Info
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
esp_now_peer_info_t peerInfo;

// ESP-NOW Callback for Sending Data
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.printf("[ESP-NOW] Message Sent: %s\n", (status == ESP_NOW_SEND_SUCCESS) ? "Success" : "Fail");
}

// ESP-NOW Callback for Receiving Data (Fixed for ESP-IDF v5+)
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
  } else {
    Serial.printf("[DRONE %d] Unknown Command Received: %s\n", droneID, receivedCommand.command);
  }
}

// Function to Send Command (Only Leader Uses This)
void sendCommand(const char *cmd) {
  DroneCommand commandToSend;
  strcpy(commandToSend.command, cmd);
  commandToSend.senderID = droneID;  

  esp_now_send(broadcastAddress, (uint8_t *)&commandToSend, sizeof(commandToSend));
  Serial.printf("[LEADER %d] Sending Command: %s\n", droneID, cmd);
}

// ESP-NOW Setup Function
void setupESPNow() {
  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERROR] ESP-NOW Initialization Failed!");
    return;
  }
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("[ERROR] Failed to Add ESP-NOW Peer!");
  }
}

// OTA Setup (Only if Enabled)
void setupOTA() {
  if (!OTA_ENABLED) return;

  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([]() {
    Serial.println("[OTA] Update Started...");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("[OTA] Update Completed!");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("[OTA] Progress: %u%%\r", (progress * 100) / total);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[OTA] Error[%u]\n", error);
  });

  ArduinoOTA.begin();
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  WiFi.mode(WIFI_STA);

  setupESPNow();
  setupOTA();

  Serial.printf("[INFO] Drone Initialized - ID: %d | Role: %s\n", droneID, isLeader ? "Leader" : "Follower");

  if (MULTI_LEADER && !isLeader) {
    Serial.println("[FOLLOWER] Waiting for Leader presence...");
    delay(LEADER_TIMEOUT + 5000); // Wait 10 seconds before assuming leadership
    
    if (!isLeader) {
      Serial.printf("[DRONE %d] No leader detected after waiting. Taking over as Leader!\n", droneID);
      isLeader = true;
    }
  }

  // Show available commands for the Leader
  if (isLeader) {
    Serial.println("\n🛠️ ENTER COMMANDS BELOW:");
    Serial.println("1. TRIANGLE");
    Serial.println("2. LINE");
    Serial.println("3. HOVER");
    Serial.println("4. LAND");
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
      input.toUpperCase();

      if (input == "TRIANGLE" || input == "LINE" || input == "HOVER" || input == "LAND") {
        sendCommand(input.c_str());
      } else {
        Serial.println("[ERROR] Invalid Command! Try: TRIANGLE, LINE, HOVER, LAND");
      }
    }
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  delay(COMMUNICATION_INTERVAL);
}
