#include "constants.h"
#include <ESP.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <FastLED.h>

// Shared application state
WiFiUDP udp;
const CRGBArray<LED_COUNT> leds;
uint8_t packetBuffer[LED_BUFFER_SIZE];
boolean connected = false;
uint32_t lastSuccessfulPacketTime = millis();
uint8_t brightness = UINT8_MAX;

void setup() {
  Serial.begin(115200);

  #if CLOCK_PIN == -1
  FastLED.addLeds<CHIPSET, DATA_PIN, RGB_ORDER>(leds, LED_COUNT).setCorrection(TypicalSMD5050);
  #else
  FastLED.addLeds<CHIPSET, DATA_PIN, CLOCK_PIN, RGB_ORDER>(leds, LED_COUNT).setCorrection(TypicalSMD5050);
  #endif

  connectToWiFi(NETWORK_SSID, NETWORK_PASSWORD);
}

void loop() {
  uint32_t ms = millis();

  if (!connected) {
    return;
  }

  if (millis() - lastSuccessfulPacketTime > MAX_CONNECTION_LOSS_MS) {
    Serial.println("No packets received in " + String(MAX_CONNECTION_LOSS_MS) + "ms, restarting.");
    ESP.restart();
    while (true);
  }

  renderFrame(STAGE_PROP_CODE, 0, LED_COUNT);

  uint16_t elapsedMs = millis() - ms;
  if (elapsedMs < MILLIS_PER_FRAME) {
    delay(MILLIS_PER_FRAME - elapsedMs);
  }
}

void renderFrame(String stagePropCode, uint16_t startLed, uint16_t endLed) {
  uint32_t ms = millis();

  if (!udp.beginPacket(SERVER_IP_ADDRESS, SERVER_UDP_PORT)) {
    Serial.println("beginPacket() failed.");
    return;
  }
  udp.printf(String(GET_FRAME_COMMAND + stagePropCode).c_str());
  udp.endPacket();

  uint16_t packetSize = 0;
  do {
    packetSize = udp.parsePacket();

    uint16_t waitMillis = millis() - ms;
    if (waitMillis > PACKET_TIMEOUT_MS) {
      Serial.println("Lost frame after " + String(PACKET_TIMEOUT_MS) + " ms, skipping.");
      return;
    }
  } while (packetSize == 0);

  udp.read(packetBuffer, LED_BUFFER_SIZE);
  adjustBrightness();
  renderLeds(packetSize, startLed, endLed);
  
  lastSuccessfulPacketTime = millis();
}

void adjustBrightness() {
  // Brightness is stored in the bottom 4 bits.
  int16_t newBrightness = map(packetBuffer[0] & 0b00001111, 0, 15, 0, UINT8_MAX);

  // The ESP32 core defines min() and max() with an underscore prefix to avoid code conflits.
  if (brightness < newBrightness) {
    brightness = _min(UINT_MAX, brightness + 1);
  } else if (brightness > newBrightness) {
    brightness = _max(0, brightness - 1);
  }

  FastLED.setBrightness(brightness);
}

void renderLeds(uint16_t packetSize, uint16_t startLed, uint16_t endLed) {
  fillLeds(startLed, endLed - startLed, CRGB::Black);
  uint16_t packetLedCount = (packetSize - HEADER_SIZE) / BYTES_PER_LED;

  for (uint16_t i = startLed; i < _min(startLed + packetLedCount, endLed); i++) {
    uint16_t bufferIndex = HEADER_SIZE + ((i - startLed) * BYTES_PER_LED);
    leds[i].setRGB(packetBuffer[bufferIndex], packetBuffer[bufferIndex + 1], packetBuffer[bufferIndex + 2]);
  }

  FastLED.show();
}

void connectToWiFi(const String ssid, const String pwd) {
  Serial.println("Connecting to network: " + ssid + "...");
  showStatus(STATUS_CONNECTING);

  WiFi.disconnect(true);
  WiFi.onEvent(WiFiEvent);

  #ifdef STATIC_IP_ADDRESS
  if (WiFi.config(IPAddress(STATIC_IP_ADDRESS), IPAddress(ROUTER_IP_ADDRESS), IPAddress(SUBNET_MASK), IPAddress(DNS_PRIMARY), IPAddress(DNS_SECONDARY))) {
    Serial.println("Using static IP address.");
  } else {
    Serial.println("Failed to configure static IP address.");
  }
  #else
  Serial.println("Using dynamic IP address.");
  #endif

  WiFi.begin(ssid.c_str(), pwd.c_str());
  Serial.println("Waiting for network connection...");
}

void showStatus(CRGB statusColor) {
  fillLeds(0, LED_COUNT, CRGB::Black);
  for (uint8_t i = 0; i < STATUS_LED_COUNT; i++) {
    leds[i] = statusColor;
  }
  FastLED.show();
}

void fillLeds(uint16_t startLed, uint16_t endLed, CRGB color) {
  for (uint16_t i = startLed; i < endLed; i++) {
    leds[i] = color;
  }

  FastLED.show();
}

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_STA_CONNECTED:
      Serial.println("Connected to network.");
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      if (WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
        Serial.println("Got IP Address of 0.0.0.0, waiting for proper IP address...");
      } else {
        if (!connected) {
          Serial.print("Connected with IP address ");
          Serial.println(WiFi.localIP());

          udp.begin(WiFi.localIP(), SERVER_UDP_PORT);
          connected = true;
          lastSuccessfulPacketTime = millis();
        }
      }
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("Lost network connection, attempting to reconnect...");
      connected = false;
      connectToWiFi(NETWORK_SSID, NETWORK_PASSWORD);
      break;
  }
}
