#include <EEPROM.h>

// Allows writing/reading of config structures direct to EEPROM
template <class T> int EWAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
        EEPROM.write(ee++, *p++);
    EEPROM.commit();
    return i;
}

template <class T> int ERAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
        *p++ = EEPROM.read(ee++);
    return i;
}

#define rebootPin 15
inline void doReboot()
{
    ESP.restart();
    while (1);
}

namespace CMDTYPE
{
    enum CMDTYPE
    {
        SETUP_DATA = 1,
        PIXEL_DATA = 2,
        BRIGHTNESS = 3,
        GETID      = 4,
        SETID      = 5,
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
    };
}

#define SPI_DATA 13
#define SPI_CLOCK 14
#define ONEWIREPIN SPI_DATA

#define FREE_RAM_BUFFER 180


// //#define GENERIC 0
// #define LPD8806 1
// #define WS2801 2
// //NEOPIXEL also known as WS2811, WS2812, WS2812B, APA104
// #define NEOPIXEL 3
// //400khz variant of above
// #define WS2811_400 4
// #define TM1809_TM1804 5
// #define TM1803 6
// #define UCS1903 7
// #define SM16716 8
// #define APA102 9
// #define LPD1886 10
// #define P9813 11

struct config_t
{
    uint8_t type;
    uint16_t pixelCount;
    uint8_t spiSpeed;
} config;
#define CONFIGCHECK 7


void writeConfig()
{
    EWAnything(1, config);
}

void readConfig()
{
    ERAnything(1, config);
}

void writeDefaultConfig()
{
    config.type = APA102;
    config.pixelCount = 64;
    config.spiSpeed = 16;
    writeConfig();
    EEPROM.write(16, 0);
}
