#ifndef CONFIG_DRONE_H
#define CONFIG_DRONE_H

const char* WIFI_SSID = "MRP";
const char* WIFI_PASSWORD = "sebigradea";

const unsigned int UDP_PORT = 4210;
#define LED_PIN 2  // Onboard LED for ESP32 boards

#define DRONE_ID 2  // Change for each drone (1, 2, 3...)

const unsigned long SLAVE_STROBE_INTERVAL = 15000;

#endif
