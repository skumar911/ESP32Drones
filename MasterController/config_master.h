#ifndef CONFIG_MASTER_H
#define CONFIG_MASTER_H

const char* WIFI_SSID = "MRP";
const char* WIFI_PASSWORD = "sebigradea";

const unsigned int UDP_PORT = 4210;
const int LED_PIN = 2;  

const unsigned long DISCOVERY_INTERVAL = 5000;
const unsigned long POLL_INTERVAL = 2000;
const unsigned long MASTER_STROBE_INTERVAL = 10000;
const int MAX_DRONES = 10;

#endif
