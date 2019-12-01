#include "constants.h"
#include <ESP.h>

#if defined(ESP8266)
    #include <ESP8266WiFi.h>
#elif defined(ESP32)
    #include <WiFi.h>
#endif

#include <WiFiUdp.h>

#ifdef OTA_UPDATES_ENABLED
    #include <ArduinoOTA.h>
#endif

#include <FastLED.h>

// Shared application state
WiFiUDP udp;
CRGB leds[STAGE_PROP_COUNT][LED_COUNT];
uint8_t packetBuffer[LED_BUFFER_SIZE];
boolean connected = false;
uint32_t lastSuccessfulPacketTime = 0;
uint32_t lastSubscribeTime = 0;
bool otaUpdating = false;

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

  #ifdef OTA_UPDATES_ENABLED
  ArduinoOTA.handle();

  if (otaUpdating) {
    return;
  }
  #endif

  checkForNetworkFailure();

  if (lastSubscribeTime == 0 || ms - lastSubscribeTime > SUBSCRIBE_INTERVAL_MS) {
    for (uint8_t i = 0; i < STAGE_PROP_COUNT; i++) {
      subscribe(STAGE_PROP_CODES[i], i);
    }

    lastSubscribeTime = ms;
  }

  receiveFrame();
}

void checkForNetworkFailure() {
  if (millis() - lastSuccessfulPacketTime > MAX_CONNECTION_LOSS_MS) {
    Serial.println("No packets received in " + String(MAX_CONNECTION_LOSS_MS) + "ms, restarting.");
    ESP.restart();
    while (true);
  }
}

void subscribe(String stagePropCode, uint8_t clientId) {
  uint32_t ms = millis();

  if (!udp.beginPacket(SERVER_IP_ADDRESS, SERVER_UDP_PORT)) {
    Serial.println("beginPacket() failed.");
    return;
  }
  udp.printf(String(SUBSCRIBE_COMMAND + stagePropCode + ":" + clientId).c_str());
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
  uint8_t brightness = map(packetBuffer[0] & 0b00001111, 0, 15, 0, UINT8_MAX);
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
  WiFi.onEvent(onWiFiEvent);

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

void onWiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case EVENT_CONNECTED:
      Serial.println("Connected to network.");
      break;
    case EVENT_GOT_IP:
      if (WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
        Serial.println("Got IP Address of 0.0.0.0, waiting for proper IP address...");
      } else {
        if (!connected) {
          Serial.print("Connected with IP address ");
          Serial.println(WiFi.localIP());

          #ifdef OTA_UPDATES_ENABLED
            ArduinoOTA
              .onStart([]() {
                otaUpdating = true;
                Serial.println("Over-the-air update has started.");
              });

            ArduinoOTA.begin();
            Serial.println("Over-the-air updater is listening.");
          #endif

          udp.begin(SERVER_UDP_PORT);
          connected = true;
          lastSuccessfulPacketTime = millis();
        }
      }
      break;
    case EVENT_DISCONNECTED:
      Serial.println("Lost network connection, attempting to reconnect...");
      connected = false;
      connectToWiFi(NETWORK_SSID, NETWORK_PASSWORD);
      break;
  }
}
