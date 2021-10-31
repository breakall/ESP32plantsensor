# ESP32plantsensor
ESP32plantsensor

Hardware:
- ESP32
- DHT22 temperature / humidity sensor
- Soil moisture sensor 1.2

Operation:
- Reads from both sensors every X seconds
- Posts the values to MQTT topics

For switching to OTA upload see comments in platformio.ini

Note that OTA uploading won't work until MQTT is connected because of the library.

Debugging will spam over UDP.  
To start the monitor, in a terminal run: node ./udpmonitor/index.js
