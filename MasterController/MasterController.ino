#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "config_master.h"

/************************************************************
   MASTER CODE (Scalable for 10+ Drones)

   🔴 Wi-Fi Connecting → Fast Blinking (100ms)
   🟢 Wi-Fi Connected → Solid ON for 2 sec
   🔵 Sending `FindDrones` → Quick Blink (300ms)
   🟡 New Drone Discovered → 2 Quick Blinks (200ms)
   ⚪ Polling Drone → Solid ON for 500ms
   🌟 Identifying Master → 5 Fast Blinks (100ms) every 10 sec
************************************************************/

struct DroneInfo {
  IPAddress ip;
  int droneID;
  bool active;
};

DroneInfo droneList[MAX_DRONES];
int discoveredCount = 0;
int currentDroneIndex = 0;

unsigned long lastPollTime = 0;
unsigned long lastDiscoveryTime = 0;
unsigned long lastStrobeTime = 0;

WiFiUDP udp;

/**
 * Function to blink the LED
 */
void blinkLED(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(delayMs);
    digitalWrite(LED_PIN, LOW);
    delay(delayMs);
  }
}

/**
 * Function to connect to Wi-Fi
 */
void setupWiFi() {
  Serial.println("[MASTER] Connecting to Wi-Fi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    blinkLED(1, 100);
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("[MASTER] Wi-Fi connected!");
  Serial.print("[MASTER] IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_PIN, HIGH);
  delay(2000);
  digitalWrite(LED_PIN, LOW);
}

/**
 * Function to setup OTA
 */
void setupOTA() {
  ArduinoOTA.setHostname("ESP32-Master");
  ArduinoOTA.onStart([]() {
    Serial.println("[MASTER] OTA Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\n[MASTER] OTA End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("[MASTER] OTA Progress: %u%%\r", (progress * 100) / total);
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[MASTER] OTA Error[%u]\n", error);
  });
  ArduinoOTA.begin();
}

/**
 * Function to setup UDP communication
 */
void setupUDP() {
  udp.begin(UDP_PORT);
  Serial.printf("[MASTER] Listening on port %d\n", UDP_PORT);
}

/**
 * Add a discovered drone to the list
 */
void addDrone(IPAddress newIP, int newID) {
  for (int i = 0; i < discoveredCount; i++) {
    if (droneList[i].droneID == newID) {
      droneList[i].ip = newIP;
      droneList[i].active = true;
      return;
    }
  }
  if (discoveredCount < MAX_DRONES) {
    droneList[discoveredCount].ip = newIP;
    droneList[discoveredCount].droneID = newID;
    droneList[discoveredCount].active = true;
    discoveredCount++;
    Serial.printf("[MASTER] Discovered Drone ID=%d at IP=%s\n", newID, newIP.toString().c_str());
    blinkLED(2, 200);
  }
}

/**
 * Function to send broadcast discovery request
 */
void sendDiscoveryRequest() {
  const char* discoveryMsg = "FindDrones";
  udp.beginPacket(IPAddress(255,255,255,255), UDP_PORT);
  udp.write((uint8_t*)discoveryMsg, strlen(discoveryMsg));
  udp.endPacket();
  Serial.println("[MASTER] Broadcast => FindDrones");
  blinkLED(1, 300);
}

/**
 * Function to poll a discovered drone
 */
void pollCurrentDrone() {
  if (discoveredCount == 0) return;
  
  currentDroneIndex = (currentDroneIndex + 1) % discoveredCount;
  if (!droneList[currentDroneIndex].active) return;

  char requestMsg[32];
  snprintf(requestMsg, sizeof(requestMsg), "ReportStatus,%d", droneList[currentDroneIndex].droneID);
  udp.beginPacket(droneList[currentDroneIndex].ip, UDP_PORT);
  udp.write((uint8_t*)requestMsg, strlen(requestMsg));
  udp.endPacket();
  Serial.printf("[MASTER] Requested data from Drone ID=%d at IP=%s\n",
                droneList[currentDroneIndex].droneID, 
                droneList[currentDroneIndex].ip.toString().c_str());

  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
}

void setup() {
  Serial.begin(115200);
  Serial.println("[DEBUG] Booting...");
  
  pinMode(LED_PIN, OUTPUT);
  setupWiFi();
  setupOTA();
  setupUDP();

  Serial.println("[DEBUG] Setup Complete!");
}

/**
 * Main loop function
 */
void loop() {
  ArduinoOTA.handle();

  int packetSize = udp.parsePacket();
  if (packetSize) {
    char incomingBuffer[256];
    int len = udp.read(incomingBuffer, sizeof(incomingBuffer) - 1);
    if (len > 0) incomingBuffer[len] = '\0';

    int droneID;
    if (sscanf(incomingBuffer, "IAmDrone,%d", &droneID) == 1) {
      addDrone(udp.remoteIP(), droneID);
      Serial.printf("[MASTER] Received IAmDrone from Drone ID=%d at %s\n", 
                    droneID, udp.remoteIP().toString().c_str());
    } 
    else {
      Serial.printf("[MASTER] Received Unrecognized Packet: %s\n", incomingBuffer);
    }
  }

  unsigned long now = millis();
  if (now - lastDiscoveryTime > DISCOVERY_INTERVAL) {
    lastDiscoveryTime = now;
    sendDiscoveryRequest();
  }
  if (now - lastPollTime > POLL_INTERVAL) {
    lastPollTime = now;
    pollCurrentDrone();
  }
  if (now - lastStrobeTime > MASTER_STROBE_INTERVAL) {
    lastStrobeTime = now;
    blinkLED(5, 100);
  }
}
