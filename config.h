#ifndef CONFIG_H
#define CONFIG_H

#include <FastLED.h>

// Network configuration.
#define NETWORK_SSID "YOUR_NETWORK_SSID"
#define NETWORK_PASSWORD "YOUR_NETWORK_PASSWORD"
#define SERVER_IP_ADDRESS "YOUR_SERVER_IP_ADDRESS"
#define SERVER_UDP_PORT 2812

// Stage prop configuration
#define STAGE_PROP_CODE "PROP_1"

// LED strip configuration
#define LED_COUNT 150

// Animation configuration
#define TARGET_FPS 60

// ESP32 configuration
#define CHIPSET WS2812B
#define CLOCK_PIN -1 // Leave as -1 if you are using a clockless chipset.
#define DATA_PIN 13
#define RGB_ORDER GRB

#endif CONFIG_H
