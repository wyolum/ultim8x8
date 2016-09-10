#include "Adafruit_DotStar.h"
#include <SPI.h>


#define NUMPIXELS 64

#define DATAPIN    23
#define CLOCKPIN   24
Adafruit_DotStar strip = Adafruit_DotStar(
    NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);
#define LEDVAL 1

void setup() {
    strip.begin(); // Initialize pins for output
    strip.show(); // Turn all LEDs off ASAP
}

void fill(uint8_t r, uint8_t g, uint8_t b) {
    for(uint8_t i=0;i<NUMPIXELS;i++)
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
    for(int ii=0; ii<64; ii++){
      //strip.setPixelColor(ii, strip.Color(ii, ii * 7, ii*17));
    }
    strip.show();
}
