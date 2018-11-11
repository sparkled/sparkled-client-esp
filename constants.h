#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "config.h"
#include <FastLED.h>

// Network constants
#define PACKET_TIMEOUT_MS 50
#define MAX_CONNECTION_LOSS_MS 5000

// Status constants
#define STATUS_LED_COUNT 3
#define STATUS_CONNECTING CRGB::Orange
#define STATUS_RESTARTING CRGB::Red

// Animation constants
#define MILLIS_PER_FRAME 1000 / TARGET_FPS

// LED strip constants
#define BYTES_PER_LED 3
#define HEADER_SIZE 1
#define LED_BUFFER_SIZE HEADER_SIZE + BYTES_PER_LED * LED_COUNT

#endif CONSTANTS_H
