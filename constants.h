#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <FastLED.h>

// Network configuration
#define NETWORK_SSID "YOUR_NETWORK"
#define NETWORK_PASSWORD "YOUR_PASSWORD"
#define SERVER_IP_ADDRESS "YOUR_SERVER_IP_ADDRESS"
#define SERVER_UDP_PORT 12345
#define MAX_PACKET_WAIT_MS 5000

// ESP32 configuration
#define CLOCK_PIN 12
#define DATA_PIN 13
#define STATUS_CONNECTING CRGB::Orange
#define STATUS_RESTARTING CRGB::Red

// Animation configuration
#define MAX_FPS 60
#define MILLIS_PER_FRAME 1000 / MAX_FPS

// LED strip configuration
#define BYTES_PER_LED 3
#define LED_COUNT 150
#define HEADER_SIZE 1
#define LED_BUFFER_SIZE HEADER_SIZE + BYTES_PER_LED * LED_COUNT
#define STATUS_LED_COUNT 3

#endif CONSTANTS_H
