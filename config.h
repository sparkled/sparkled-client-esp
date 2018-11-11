#ifndef CONFIG_H
#define CONFIG_H

#include <FastLED.h>

// Network configuration.
#define NETWORK_SSID "YOUR_NETWORK_SSID"
#define NETWORK_PASSWORD "YOUR_NETWORK_PASSWORD"
#define SERVER_IP_ADDRESS "YOUR_SERVER_IP_ADDRESS"
#define SERVER_UDP_PORT 12345

// Stage prop configuration
#define STAGE_PROP_CODE "PROP_1"

// LED strip configuration
#define LED_COUNT 150

// Animation configuration
#define TARGET_FPS 60

// ESP32 configuration
#define CLOCK_PIN 12
#define DATA_PIN 13

#endif CONFIG_H
