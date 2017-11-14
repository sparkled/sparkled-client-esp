#include <WiFi.h>
#include <WiFiUdp.h>
#include <FastLED.h>

// WiFi network name and password:
const char * networkName = "YOUR_NETWORK";
const char * networkPswd = "YOUR_PASSWORD";

const char * udpAddress = "YOUR_UDP_ADDRESS";
const int udpPort = 12345;

#define MAX_FPS 60
#define MILLIS_PER_FRAME 1000 / MAX_FPS

#define DATA_PIN 18
#define CLOCK_PIN 19
#define LED_COUNT 50
#define BYTES_PER_LED 3
#define LED_BUFFER_SIZE LED_COUNT * BYTES_PER_LED

uint8_t packetBuffer[LED_BUFFER_SIZE];

boolean connected = false;
WiFiUDP udp;
CRGBArray<LED_COUNT> leds;

void setup() {
  Serial.begin(115200);
  pinMode(DATA_PIN, OUTPUT);
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN>(leds, LED_COUNT).setCorrection(TypicalSMD5050);

  //Connect to the WiFi network
  connectToWiFi(networkName, networkPswd);
}

void loop() {
  long ms = millis();

  if (!connected) {
    return;
  }

  Serial.println("Starting packet.");
  if (!udp.beginPacket(udpAddress, udpPort)) {
    Serial.println("Failed to begin packet.");
    delay(500);
    return;
  }
  udp.printf("GF:P1");
  udp.endPacket();

  int packetSize = 0;
  do {
    packetSize = udp.parsePacket();

    if (millis() - ms > 5 * MILLIS_PER_FRAME) {
      Serial.println("Lost frame, skipping.");
      return; // Lost frame, skip.
    }
  } while (packetSize == 0);

  Serial.print("Packet size: ");
  Serial.println(packetSize);

  udp.read(packetBuffer, LED_BUFFER_SIZE);

  Serial.println("Read into packetBuffer.");
  renderLeds();
//  Serial.println("Rendered LEDs.");

  int elapsedMs = millis() - ms;
  if (elapsedMs < MILLIS_PER_FRAME) {
    delay(MILLIS_PER_FRAME - elapsedMs);
  }
}

void clearLeds() {
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i] = CRGB::Black;
  }

  FastLED.show();
}

void renderLeds() {
  for (int i = 0; i < LED_COUNT; i++) {
    int bufferIndex = i * BYTES_PER_LED;
    leds[i].setRGB(packetBuffer[bufferIndex], packetBuffer[bufferIndex + 1], packetBuffer[bufferIndex + 2]);
  }

  FastLED.show();
}

void connectToWiFi(const char * ssid, const char * pwd) {
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);

  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event) {
  Serial.println("Event: " + event);
  switch (event) {
    case SYSTEM_EVENT_STA_CONNECTED:
      Serial.println("Connected, waiting for IP address.");
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      if (WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
        Serial.println("Got 0.0.0.0");
        return;
      }
      //When connected set
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      //initializes the UDP state
      //This initializes the transfer buffer
      udp.begin(WiFi.localIP(), udpPort);
      connected = true;
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection, attempting to reconnect.");
      connected = false;
      connectToWiFi(networkName, networkPswd);
      break;
    default:
      Serial.print("Some other event.");
      Serial.println(event);
      delay(100);
      break;
  }
}
