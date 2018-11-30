#ifndef CONFIG_H
#define CONFIG_H

#include <FastLED.h>

// Network configuration
#define NETWORK_SSID "YOUR_NETWORK_SSID"
#define NETWORK_PASSWORD "YOUR_NETWORK_PASSWORD"
#define SERVER_IP_ADDRESS IPAddress(192, 168, 0, 2)
#define SERVER_UDP_PORT 2812

// Static IP configuration (optional)
 #define ROUTER_IP_ADDRESS IPAddress(192, 168, 0, 1)
 #define STATIC_IP_ADDRESS IPAddress(192, 168, 0, 3)
 #define SUBNET_MASK IPAddress(255, 255, 255, 0)
 #define DNS_PRIMARY IPAddress(1, 1, 1, 1)
 #define DNS_SECONDARY IPAddress(8, 8, 8, 8)

// LED strip configuration
#define STAGE_PROP_COUNT 1
#define LED_COUNT 150

// Stage prop configuration
const String STAGE_PROP_CODES[STAGE_PROP_COUNT] = {"PROP_1"};

// Animation configuration
#define TARGET_FPS 60

// ESP32 configuration
#define CHIPSET WS2812B
#define RGB_ORDER GRB
#define CLOCK_PIN -1 // Leave as -1 if you are using a clockless chipset.
#define DATA_PIN 19

#endif CONFIG_H
