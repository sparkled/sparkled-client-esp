# sparkled-client-esp
This is the official ESP32 / ESP8266 client for Sparkled.

# Requirements
- An ESP32 or ESP8266 board.
- The latest [Arduino IDE](https://www.arduino.cc/en/Main/Software) with
  [ESP32 board definitions](https://github.com/espressif/arduino-esp32#installation-instructions) or
  [ESP8266 board definitions](https://arduino-esp8266.readthedocs.io/en/2.5.2/installing.html#boards-manager) installed.
- The latest [FastLED](http://fastled.io) version.

# Installation instructions
1. Clone this repository (preferred), or download a zipped copy.
2. Open the `.ino` file in the Arduino IDE to open the project.
3. Edit the `config.h` file to match your LED strip, network and Sparkled stage configuration.
4. Change the board in the Arduino settings to match your ESP board.
5. Build the project to install the client onto your ESP.

For more information, see [the main Sparkled repository](https://github.com/sparkled/sparkled).
