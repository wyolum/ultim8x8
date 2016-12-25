#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESPSerialWiFiManager.h>
#include <FastLED.h>
#include <EEPROM.h>
#include "global.h"

ESPSerialWiFiManager esp = ESPSerialWiFiManager();

WiFiUDP udp;
IPAddress remote_ip;
int remote_port;

#define numLEDs 64

#define BASE_BRIGHTNESS 64 //battery cannot handle full
#define STATUS_LEVEL 32
#define RED {STATUS_LEVEL, 0, 0}
#define GREEN {0, STATUS_LEVEL, 0}
#define BLUE {0, 0, STATUS_LEVEL}

bool errorPixelCount = false;
uint8_t bytesPerPixel = 3;

CRGB leds[numLEDs];

int data_available(){
    static int avail = 0;
    avail = udp.parsePacket();
    if(avail) {
        remote_ip = udp.remoteIP();
        remote_port = udp.remotePort();
    }
    return avail;
}

void begin_udp_listen(){
    udp.begin(1822);
}

int data_read(uint8_t* buf, int len){
    return udp.read(buf, len);
}

int data_read(){
    return udp.read();
}

size_t data_write(uint8_t* buf, int len){
    size_t resp;
    udp.beginPacket(remote_ip, remote_port);
    resp = udp.write(buf, len);
    udp.endPacket();
    return resp;
}

size_t data_write(uint8_t buf){
    size_t resp;
    udp.beginPacket(remote_ip, remote_port);
    resp = udp.write(buf);
    udp.endPacket();
    return resp;
}

void data_flush(){
    udp.flush();
}

void data_stop(){
    udp.stop();
}

void display_network_status(){
    IPAddress ip = WiFi.localIP();
    fill_solid(leds+10, 4, RED);
    fill_solid(leds+50, 4, BLUE);
    CRGB c = GREEN;

    uint8_t b = 0;
    for(int s=0; s<4; s++){
        for(int i=0; i<8; i++){
            b = (s % 2 == 0) ? 7-i : i;
            leds[16 + 8*s + b] = bool(ip[s] & _BV(i)) ? c : CRGB::Black;
        }
    }
    FastLED.delay(10);
}

void fill_color(CRGB color){
    fill_solid(leds, numLEDs, color);
    FastLED.show();
    FastLED.delay(0);
}

void corner_color(CRGB color){
    leds[0] =
    leds[7] =
    leds[56] =
    leds[63] = color;

    FastLED.show();
    FastLED.delay(0);
}

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(false);
    Serial.setTimeout(1000);

    OL("\n\n");

    // Setting channel order here because this outputs status on boot
    FastLED.addLeds<APA102, SPI_DATA, SPI_CLOCK, BGR>(leds, numLEDs);
    FastLED.setBrightness(BASE_BRIGHTNESS);
    FastLED.clear();
    FastLED.show();

    corner_color(BLUE);

    esp.begin();
    esp.run_menu(10);
    NL();
    OL("WiFi Config Complete");

    if(esp.status() == WL_CONNECTED){
        display_network_status();
        corner_color(GREEN);
    }
    else{
        corner_color(RED);
    }

    begin_udp_listen();
    FastLED.delay(10);
}

#define EMPTYMAX 100
inline void getData()
{
    static char cmd = 0;
    static uint16_t size = 0;
    static uint16_t count = 0;
    static uint8_t emptyCount = 0;
    static int available_data = 0;
    static size_t c = 0;
    static uint16_t packSize = numLEDs * bytesPerPixel;
    static int counter = 0;
    available_data = data_available();
    if(available_data){
        cmd = data_read();
        size = 0;
        data_read((uint8_t*)&size, 2);

        if (cmd == CMDTYPE::PIXEL_DATA)
        {
            counter ++;
            count = 0;
            emptyCount = 0;

            if (size == packSize)
            {
                while (count < packSize)
                {
                    c = data_read(((uint8_t*)&leds) + count, 64);
                    if (c == 0)
                    {
                        emptyCount++;
                        if(emptyCount > EMPTYMAX) break;
                    }
                    else
                    {
                        emptyCount = 0;
                    }

                    count += c;
                }
            }

            if (count == packSize)
            {
                FastLED.show();
                FastLED.delay(0);
            }
            else{
                Serial.println("BPS");
            }
        }

        // data_write(resp);
        // data_flush();
        // data_stop();
    }
}

void loop()
{
    // Serial.println("Loop");
    getData();
}
