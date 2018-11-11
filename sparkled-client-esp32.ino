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

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, LED_COUNT).setCorrection(TypicalSMD5050);
  connectToWiFi(NETWORK_SSID, NETWORK_PASSWORD);
}

void loop() {
  uint32_t ms = millis();

  if (!connected) {
    return;
  }

  if (millis() - lastSuccessfulPacketTime > MAX_PACKET_WAIT_MS) {
    Serial.println("No packets received in " + String(MAX_PACKET_WAIT_MS) + "ms, restarting.");
    showStatus(STATUS_RESTARTING);
    delay(1000);
    ESP.restart();
    while (true);
  }

  if (!udp.beginPacket(SERVER_IP_ADDRESS, SERVER_UDP_PORT)) {
    Serial.println("beginPacket() failed.");
    return;
  }
  udp.printf("GF:P1");
  udp.endPacket();

  uint16_t packetSize = 0;
  do {
    packetSize = udp.parsePacket();

    uint16_t waitMillis = millis() - ms;
    if (waitMillis > 3 * MILLIS_PER_FRAME) {
      Serial.println("Lost frame after " + String(waitMillis) + " ms, skipping.");
      return;
    }
  } while (packetSize == 0);

  udp.read(packetBuffer, LED_BUFFER_SIZE);
  lastSuccessfulPacketTime = millis();

  if (packetSize == LED_BUFFER_SIZE) {
    renderLeds();
  } else {
    Serial.println("packetSize does not equal LED_BUFFER_SIZE, skipping.");
    fillLeds(CRGB::Black);
  }

  uint16_t elapsedMs = millis() - ms;
  if (elapsedMs < MILLIS_PER_FRAME) {
    delay(MILLIS_PER_FRAME - elapsedMs);
  }
}

void renderLeds() {
  for (uint16_t i = 0; i < LED_COUNT; i++) {
    uint16_t bufferIndex = HEADER_SIZE + (i * BYTES_PER_LED);
    leds[i].setRGB(packetBuffer[bufferIndex], packetBuffer[bufferIndex + 1], packetBuffer[bufferIndex + 2]);
  }

  FastLED.show();
}

void connectToWiFi(const String ssid, const String pwd) {
  Serial.println("Connecting to network: " + ssid + "...");
  showStatus(STATUS_CONNECTING);

  WiFi.disconnect(true);
  WiFi.onEvent(WiFiEvent);
  WiFi.begin(ssid.c_str(), pwd.c_str());

  Serial.println("Waiting for network connection...");
}

void showStatus(CRGB statusColor) {
  fillLeds(CRGB::Black);
  for (uint8_t i = 0; i < STATUS_LED_COUNT; i++) {
    leds[i] = statusColor;
  }
  FastLED.show();
}

void fillLeds(CRGB color) {
  fill_solid(leds, LED_COUNT, color);
  FastLED.show();
}

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_STA_CONNECTED:
      Serial.println("Connected to network, waiting for IP address...");
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      if (WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
        Serial.println("Got IP Address of 0.0.0.0");
      } else {
        Serial.println("IP address: " + WiFi.localIP());
        udp.begin(WiFi.localIP(), SERVER_UDP_PORT);
        connected = true;
        lastSuccessfulPacketTime = millis();
      }
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("Lost network connection, attempting to reconnect...");
      connected = false;
      connectToWiFi(NETWORK_SSID, NETWORK_PASSWORD);
      break;
  }
}
