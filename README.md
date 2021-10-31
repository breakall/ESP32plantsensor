# ESP32plantsensor
ESP32plantsensor

For switching to OTA upload see comments in platformio.ini

Note that OTA uploading won't work until MQTT is connected because of the library.

Debugging will spam over UDP.  
To start the monitor, in a terminal run: node ./udpmonitor/index.js