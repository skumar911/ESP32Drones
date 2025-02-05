#ifndef CONFIG_H
#define CONFIG_H

#define LED_PIN 2  // Onboard LED for visual cues

// 🛠️ GENERAL SETTINGS
#define OTA_ENABLED false  // Enable OTA updates (true/false)
#define MULTI_LEADER true  // Allow multiple leaders

// 🛠️ DRONE IDENTIFICATION
#define DRONE_ID -1  // -1 for auto-assign, set a number for manual ID

// 🛠️ LEADERSHIP CONFIG
#define IS_LEADER false  // Set to true for manually assigned leaders
#define LEADER_TIMEOUT 5000  // Time to wait before auto-assigning a leader (ms)

// 🛠️ ESP-NOW COMMUNICATION
#define MAX_DRONES 20  
#define COMMUNICATION_INTERVAL 500  

// 🛠️ OTA UPDATES
#define OTA_HOSTNAME "DroneSwarm"
#define OTA_PASSWORD "1234"

#endif
