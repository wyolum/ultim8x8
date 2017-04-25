/**********************************
Generic template for creating *duino firmware that works with
BiblioPixel DriverSerial. Currently assumes the use of FastLED
but could be reworked for other libraries.
If using "SmartMatrix" style panels checkout:
https://github.com/ManiacalLabs/BiblioPixelSmartMatrix
**********************************/
#include "FastLED.h"

/****************************
All Firmware options go here!
****************************/
// Uncomment below line if EEPROM available on your board
// #define USE_EEPROM

// If not using EEPROM, you can specify device ID here
#ifndef USE_EEPROM
    const uint8_t deviceID = 0;
#endif

// How many leds in your strip?
#define NUM_LEDS 8 * 8 

#define DATA_PIN MOSI
#define CLOCK_PIN SCK

#define LED_TYPE APA102
#define COLOR_ORDER BGR

CRGB leds[NUM_LEDS]; // Define the array of leds
/***************************
End Firmware options
***************************/

#ifdef USE_EEPROM
    #include <EEPROM.h>
#endif

#define FIRMWARE_VER 3

namespace CMDTYPE
{
    enum CMDTYPE
    {
        SETUP_DATA = 1,
        PIXEL_DATA = 2,
        BRIGHTNESS = 3,
        GETID      = 4,
        SETID      = 5,
        GETVER       = 6
    };
}

namespace RETURN_CODES
{
    enum RETURN_CODES
    {
        SUCCESS = 255,
        REBOOT = 42,
        ERROR = 0,
        ERROR_SIZE = 1,
        ERROR_UNSUPPORTED = 2,
        ERROR_PIXEL_COUNT = 3,
        ERROR_BAD_CMD = 4,
    };
}

typedef struct __attribute__((__packed__))
{
	uint8_t type;
	uint16_t pixelCount;
	uint8_t spiSpeed;
} config_t;

uint16_t numLEDs = NUM_LEDS;
uint8_t bytesPerPixel = 3;

inline void setupFastLED()
{
    //There are many options that could be used here.
    //Checkout the FastLED blink example or their documentation for more:
    // https://github.com/FastLED/FastLED/blob/master/examples/Blink/Blink.ino
    //But one of the below should work for standard one or two pin LEDs
    //Just change the config options above

    //Data and Clock LEDs
    FastLED.addLeds<LED_TYPE, CLOCK_PIN, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);

    //Data only LEDs
    //FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);

    FastLED.clear();
    FastLED.show();
}

void setup()
{
    // Full USB 1.1 speed, assuming your chip has native USB
    Serial.begin(12000000);
    Serial.setTimeout(1000);

    setupFastLED();
}

#define EMPTYMAX 100
inline void getData()
{
    static char cmd = 0;
    static uint16_t size = 0;
    static uint16_t count = 0;
    static uint8_t emptyCount = 0;
    static size_t c = 0;
    static uint16_t packSize = numLEDs * bytesPerPixel;

    if (Serial.available())
    {
        cmd = Serial.read();
        size = 0;
        Serial.readBytes((char*)&size, 2);

        if (cmd == CMDTYPE::PIXEL_DATA)
        {
            count = 0;
            emptyCount = 0;

            if (size == packSize)
            {
                while (count < packSize)
                {
                    c = Serial.readBytes(((char*)&leds) + count, packSize - count);
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

            uint8_t resp = RETURN_CODES::SUCCESS;
            if (count == packSize)
            {
                FastLED.show();
            }
            else
                resp = RETURN_CODES::ERROR_SIZE;

            Serial.write(resp);
        }
        else if(cmd == CMDTYPE::GETID)
        {
            #ifdef USE_EEPROM
                Serial.write(EEPROM.read(16));
            #else
                Serial.write(deviceID);
            #endif
        }
        else if(cmd == CMDTYPE::SETID)
        {
            if(size != 1)
            {
                Serial.write(RETURN_CODES::ERROR_SIZE);
            }
            else
            {
                uint8_t id = Serial.read();
                #ifdef USE_EEPROM
                    EEPROM.write(16, id);
                #endif
                Serial.write(RETURN_CODES::SUCCESS);
            }
        }
        else if (cmd == CMDTYPE::SETUP_DATA)
        {
            // for(int i=0; i<size; i++) Serial.read();

            uint8_t result = RETURN_CODES::SUCCESS;
            config_t temp;

            if (size != sizeof(config_t))
            {
                result = RETURN_CODES::ERROR_SIZE;
            }
            else
            {
                size_t read = Serial.readBytes((char*)&temp, sizeof(config_t));
                if (read != size)
                {
                    result = RETURN_CODES::ERROR_SIZE;
                }
                else
                {
                    if(temp.pixelCount / bytesPerPixel != NUM_LEDS)
                        result = RETURN_CODES::ERROR_PIXEL_COUNT;
                }
            }

            Serial.write(result);
        }
        else if (cmd == CMDTYPE::BRIGHTNESS)
        {
            uint8_t result = RETURN_CODES::SUCCESS;
            if (size != 1)
                result = RETURN_CODES::ERROR_SIZE;
            else
            {
                uint8_t brightness = 255;
                size_t read = Serial.readBytes((char*)&brightness, 1);
                if (read != size)
                    result = RETURN_CODES::ERROR_SIZE;
                else
                {
                    FastLED.setBrightness(brightness);
                }
            }

            Serial.write(result);
        }
        else if (cmd == CMDTYPE::GETVER)
        {
            Serial.write(RETURN_CODES::SUCCESS);
            Serial.write(FIRMWARE_VER);
        }
        else
        {
            Serial.write(RETURN_CODES::ERROR_BAD_CMD);
        }


        Serial.flush();
    }
}

void loop()
{
    getData();
    FastLED.delay(0);
}
