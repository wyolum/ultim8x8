#include "Adafruit_DotStar.h"
#include <SPI.h>


const uint16_t NUMPIXELS = 64*14;
#define M0

#ifdef M0
#define DATAPIN    23
#define CLOCKPIN   24
#else
#define DATAPIN    4
#define CLOCKPIN   5
#endif

#define LEDVAL 1

Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS,
					  DATAPIN,
					  CLOCKPIN,
					  DOTSTAR_BGR);

void setup() {
    strip.begin(); // Initialize pins for output
    strip.show(); // Turn all LEDs off ASAP
}

void fill(uint8_t r, uint8_t g, uint8_t b) {
    for(uint16_t i=0;i<NUMPIXELS;i++)
        strip.setPixelColor(i, r, g, b);
}

void loop() {
    fill(LEDVAL, 0, 0);
    strip.show();
    delay(500);
    fill(0, LEDVAL, 0);
    strip.show();
    delay(500);
    fill(0, 0, LEDVAL);
    strip.show();
    delay(500);
    fill(LEDVAL, LEDVAL, LEDVAL);
    strip.show();
    delay(500);
    fill(0, 0, 0);
    strip.show();
    delay(500);
}
