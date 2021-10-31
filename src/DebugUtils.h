#include <Arduino.h>
#include <stdarg.h>
#include <Print.h>
#include <WiFiUdp.h>
#include <Wifi.h>



#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H

#define DEBUG_UDP_PORT 42434

#define PRINTF_BUF 200

class SerialPrint : public Print {
    public:
        SerialPrint() : Print() 
        { }
        virtual ~SerialPrint() {}
        
        size_t write(const uint8_t *buffer, size_t size)
        {
            return Serial.write((const char*)buffer, size);
        }
        size_t write(uint8_t a) {
          return write(&a, 1);
        }

        void printf(const char *format, ...)
        {
          char buf[PRINTF_BUF];
          va_list ap;
          va_start(ap, format);
          int len = vsnprintf(buf, sizeof(buf), format, ap);
          write((const uint8_t*)buf, len );
          va_end(ap);
        }
};



const int udpPort = DEBUG_UDP_PORT;
class UdpPrint : public Print {
    private: 
    WiFiUDP udp;
    IPAddress multicastAddress = IPAddress(255, 255, 255, 255);
    bool wifiReady;


    public:
        UdpPrint() : Print() 
        {  }
        virtual ~UdpPrint() {}
        void wifiIsReady() {
            udp.begin(udpPort);
            wifiReady = true;
        }
        size_t write(const uint8_t *buffer, size_t size)
        {
            if (wifiReady) {
                IPAddress ip = WiFi.localIP();
                IPAddress bcast = IPAddress(ip[0], ip[1], ip[2], 255);

                udp.beginPacket(bcast, udpPort);
                udp.write(buffer, size);
                udp.endPacket();
            }
            return Serial.write((const char*)buffer, size);
        }
        size_t write(uint8_t) {
            return 1;
        }

};
UdpPrint dbg;

#endif



