#include "constants.h"
#include <ESP.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <FastLED.h>

// Shared application state
WiFiUDP udp;
CRGB leds[STAGE_PROP_COUNT][LED_COUNT];
uint8_t packetBuffer[LED_BUFFER_SIZE];
boolean connected = false;
uint32_t lastSuccessfulPacketTime = millis();
uint8_t brightness = UINT8_MAX;

void setup() {
  Serial.begin(115200);

  #if CLOCK_PIN == -1
  FastLED.addLeds<CHIPSET, DATA_PIN, RGB_ORDER>(leds[0], TOTAL_LED_COUNT).setCorrection(TypicalSMD5050);
  #else
  FastLED.addLeds<CHIPSET, DATA_PIN, CLOCK_PIN, RGB_ORDER>(leds[0], TOTAL_LED_COUNT).setCorrection(TypicalSMD5050);
  #endif

  connectToWiFi(NETWORK_SSID, NETWORK_PASSWORD);
}

void loop() {
  uint32_t ms = millis();

  if (!connected) {
    return;
  }

  checkForNetworkFailure();

  for (uint8_t i = 0; i < STAGE_PROP_COUNT; i++) {
    requestFrame(STAGE_PROP_CODES[i], i);
  }

  while (millis() - ms < MILLIS_PER_FRAME) {
    receiveFrame();
  }
}

void checkForNetworkFailure() {
  if (millis() - lastSuccessfulPacketTime > MAX_CONNECTION_LOSS_MS) {
    Serial.println("No packets received in " + String(MAX_CONNECTION_LOSS_MS) + "ms, restarting.");
    ESP.restart();
    while (true);
  }
}

void requestFrame(String stagePropCode, uint8_t clientId) {
  uint32_t ms = millis();

  if (!udp.beginPacket(SERVER_IP_ADDRESS, SERVER_UDP_PORT)) {
    Serial.println("beginPacket() failed.");
    return;
  }
  udp.printf(String(GET_FRAME_COMMAND + stagePropCode + ":" + clientId).c_str());
  udp.endPacket();
}

void receiveFrame() {
  uint32_t ms = millis();

  uint16_t packetSize = udp.parsePacket();
  if (packetSize > 0) {
    udp.read(packetBuffer, LED_BUFFER_SIZE);
    adjustBrightness();
    renderLeds(packetSize);
    lastSuccessfulPacketTime = millis(); 
  }
}

void adjustBrightness() {
  // Brightness is stored in the bottom 4 bits.
  int16_t newBrightness = map(packetBuffer[0] & 0b00001111, 0, 15, 0, UINT8_MAX);

  // The ESP32 core defines min() and max() with an underscore prefix to avoid code conflits.
  if (brightness < newBrightness) {
    brightness = _min(UINT8_MAX, brightness + 1);
    Serial.print("Brightness increased to " + String(brightness));
  } else if (brightness > newBrightness) {
    brightness = _max(0, brightness - 1);
    Serial.print("Brightness decreased to " + String(brightness));
  }

  FastLED.setBrightness(brightness);
}

void renderLeds(uint16_t packetSize) {
  // Client ID is stored in the top 4 bits.
  uint8_t clientId = (packetBuffer[0] & 0b11110000) >> 4;

  fillLeds(clientId, CRGB::Black);
  uint16_t packetLedCount = (packetSize - HEADER_SIZE) / BYTES_PER_LED;

  for (uint16_t i = 0; i < _min(packetLedCount, LED_COUNT); i++) {
    uint16_t bufferIndex = HEADER_SIZE + (i * BYTES_PER_LED);
    leds[clientId][i].setRGB(packetBuffer[bufferIndex], packetBuffer[bufferIndex + 1], packetBuffer[bufferIndex + 2]);
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
  for (uint8_t clientId = 0; clientId < STAGE_PROP_COUNT; clientId++) {
    fillLeds(clientId, CRGB::Black);

    for (uint8_t i = 0; i < STATUS_LED_COUNT; i++) {
      leds[clientId][i] = statusColor;
    }  
  }

  FastLED.show();
}

void fillLeds(uint8_t clientId, CRGB color) {
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    leds[clientId][i] = color;
  }
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
