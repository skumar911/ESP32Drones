#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "config_slave.h"

/************************************************************
   DRONE CODE (Scalable for 10+ Drones + OTA Updates)

   🔴 Wi-Fi Connecting → Fast Blinking (100ms)
   🟢 Wi-Fi Connected → Solid ON for 2 sec
   🔵 Receiving `FindDrones` → Quick Blink (300ms)
   🟡 Sending `IAmDrone` → 2 Quick Blinks (200ms)
   ⚪ Receiving `ReportStatus` → Solid ON for 500ms
   🌟 Identifying Drone → 3 Fast Blinks (100ms) every 15 sec
************************************************************/

WiFiUDP udp;
IPAddress masterIP;  // Store Master IP once detected
unsigned long lastStrobeTime = 0;

/**
 * Function to blink the LED a given number of times with a delay
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
  Serial.println("[INFO] Connecting to Wi-Fi...");
  Serial.flush();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttemptTime = millis();  // Start time for timeout handling

  // Loop until Wi-Fi is connected or timeout occurs (20 sec)
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) {
    Serial.println("[INFO] Waiting for Wi-Fi...");
    Serial.flush();
    blinkLED(1, 100);  // Fast blink while connecting
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[SUCCESS] Wi-Fi Connected!");
    Serial.println("[INFO] IP Address: " + WiFi.localIP().toString());
    Serial.flush();
    
    digitalWrite(LED_PIN, HIGH);  // LED ON to indicate connection success
    delay(2000);
    digitalWrite(LED_PIN, LOW);
  } else {
    Serial.println("[ERROR] Wi-Fi Connection Timeout! Check credentials or signal strength.");
    Serial.flush();
  }
}

/**
 * Function to setup OTA (Over-the-Air) updates
 */
void setupOTA() {
  char hostname[32];
  snprintf(hostname, sizeof(hostname), "Drone-%d", DRONE_ID);
  ArduinoOTA.setHostname(hostname);

  ArduinoOTA.onStart([]() {
    Serial.println("[OTA] Update Start");
    Serial.flush();
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("[OTA] Update Complete!");
    Serial.flush();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("[OTA] Progress: %u%%\r", (progress * 100) / total);
    Serial.flush();
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[OTA] Error[%u]\n", error);
    Serial.flush();
  });

  ArduinoOTA.begin();
}

/**
 * Function to initialize UDP communication
 */
void setupUDP() {
  udp.begin(UDP_PORT);
  Serial.println("[INFO] UDP Communication Initialized.");
  Serial.flush();
}

/**
 * Main setup function
 */
void setup() {
  Serial.begin(115200);
  delay(100);  // Small delay to stabilize Serial output
  Serial.println("[DEBUG] Booting up...");
  Serial.flush();

  pinMode(LED_PIN, OUTPUT);  // Ensure LED pin is set as OUTPUT

  setupWiFi();
  Serial.println("[INFO] Wi-Fi Setup Completed.");
  Serial.flush();

  setupOTA();
  Serial.println("[INFO] OTA Setup Completed.");
  Serial.flush();

  setupUDP();
  Serial.println("[INFO] UDP Setup Completed.");
  Serial.flush();

  Serial.println("[INFO] System Ready!");
  Serial.flush();
}

/**
 * Function to respond to Master
 */
void respondToFindDrones(IPAddress masterIP) {
  Serial.println("[ACTION] Responding to FindDrones command.");
  Serial.flush();

  char msgOut[32];
  snprintf(msgOut, sizeof(msgOut), "IAmDrone,%d", DRONE_ID);
  
  // Explicitly send response to the Master IP
  udp.beginPacket(masterIP, UDP_PORT);
  udp.write((uint8_t*)msgOut, strlen(msgOut));
  udp.endPacket();
  
  Serial.printf("[INFO] Sent IAmDrone Response to Master at IP: %s\n", masterIP.toString().c_str());
  Serial.flush();

  blinkLED(2, 200);  // Indicate response with two quick blinks
}

/**
 * Function to respond to status request
 */
void respondToReportStatus(IPAddress masterIP) {
  Serial.println("[ACTION] Sending Status Report.");
  Serial.flush();

  char response[128];
  snprintf(response, sizeof(response), "Drone %d: Location: 37.7749,-122.4194 | Battery: 85%% | Altitude: 150m", DRONE_ID);
  
  udp.beginPacket(masterIP, UDP_PORT);
  udp.write((uint8_t*)response, strlen(response));
  udp.endPacket();

  Serial.printf("[INFO] Sent ReportStatus Response to Master at IP: %s\n", masterIP.toString().c_str());
  Serial.flush();

  digitalWrite(LED_PIN, HIGH);  // LED ON for 500ms to indicate response
  delay(500);
  digitalWrite(LED_PIN, LOW);
}

/**
 * Main loop function
 */
void loop() {
  ArduinoOTA.handle();  // Handle OTA updates

  int packetSize = udp.parsePacket();
  if (packetSize) {
    char incoming[128];
    int len = udp.read(incoming, sizeof(incoming) - 1);
    if (len > 0) incoming[len] = '\0';

    Serial.printf("[UDP] Received Packet: %s\n", incoming);
    Serial.flush();

    IPAddress senderIP = udp.remoteIP();  // Store sender's IP for response
    Serial.printf("[INFO] Packet received from: %s\n", senderIP.toString().c_str());
    Serial.flush();

    if (strcmp(incoming, "FindDrones") == 0) {
      masterIP = senderIP;  // Update Master IP if needed
      respondToFindDrones(masterIP);
    } else if (strncmp(incoming, "ReportStatus", 12) == 0) {
      respondToReportStatus(masterIP);
    }
  }

  // Strobe Light Every 15 Sec for Identification
  if (millis() - lastStrobeTime > SLAVE_STROBE_INTERVAL) {
    lastStrobeTime = millis();
    Serial.println("[INFO] Identification Strobe Activated.");
    Serial.flush();
    blinkLED(3, 100);  // Strobe effect: 3 quick blinks
  }
}
