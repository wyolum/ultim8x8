#include "font8x16.h"
#include "Adafruit_DotStar.h"
#include <SPI.h>


const byte N_ROW = 16;
const byte N_COL = 8;
const uint16_t NUMPIXELS = 768; //N_ROW * N_COL;

#define DATAPIN    23
#define CLOCKPIN   24
Adafruit_DotStar strip = Adafruit_DotStar(
    NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);
#define LEDVAL 1

void setup() {
    strip.begin(); // Initialize pins for output
    strip.show(); // Turn all LEDs off ASAP
}

uint32_t snake(byte row, byte col){
  uint32_t out = row * N_COL;
  if(row % 2){
    out += N_COL - 1 - col;
  }
  else{
    out += col;
  }
  return out;
}

void setPixel(byte row, byte col, uint32_t color){
  strip.setPixelColor(snake(row, col), color);
}

void display_char(byte ascii, uint32_t color){
  byte *data = font_8x16 + ascii * 16;
  for(uint8_t row=0; row<16; row++){
    for(uint8_t col=0; col<8; col++){
      if((data[row] >> (7 - col)) & 1){
	setPixel(row, col, color);
      }
      else{
	setPixel(row, col, 0);
      }
    }
  }
}
		  
void fill(uint8_t r, uint8_t g, uint8_t b) {
  for(uint8_t i=0;i<NUMPIXELS;i++){
    strip.setPixelColor(i, r, g, b);
  }
}

int count = 32;
void loop() {
  display_char(count%128, strip.Color(1, 1, 1));
    strip.show();
    delay(1000);
    count++;
}
