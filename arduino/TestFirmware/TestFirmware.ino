#include "Adafruit_DotStar.h"
#include <SPI.h>


<<<<<<< HEAD
const uint16_t NUMPIXELS = 64*12;
=======
#define NUMPIXELS 384
>>>>>>> 02cda4e0ef59d59f6cc80f9263223923a69a3fd6

#define DATAPIN    4
#define CLOCKPIN   5
Adafruit_DotStar strip = Adafruit_DotStar(
    NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);
#define LEDVAL 1

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
    strip.show();
}
