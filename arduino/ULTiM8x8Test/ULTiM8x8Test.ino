#include <FastLED.h>
#include <SPI.h>

CRGB primary_colors[6] = {
  CRGB(255, 0, 0),
  CRGB(0, 255, 0),
  CRGB(0, 0, 255),
  CRGB(255, 255, 0),
  CRGB(0, 255, 255),
  CRGB(255, 0, 255),
};

const byte N_ROW = 8;
const byte N_COL = 8;
const uint16_t NUMPIXELS = N_ROW * N_COL;
CRGB leds[NUMPIXELS];

int32_t snake(byte row, byte col){
  return ((col % 2 == 0) * (col * 8 + 7 - row) +
	  (col % 2 == 1) * (col * 8 + row));
}

void setPixel(uint8_t row, uint8_t col, const struct CRGB & color){
  if(row >= N_ROW){
  }
  else if(col >= N_COL){
  }
  else{
    uint16_t pos = snake(row, col);
    leds[pos] = color;
  }
}

uint8_t brightness = 4;

void setup() {
  Serial.begin(115200);
  FastLED.setBrightness(brightness);
  FastLED.addLeds<APA102, SCK, MOSI, BGR, DATA_RATE_MHZ(25)>(leds, NUMPIXELS);
  FastLED.show(); // Turn all LEDs off ASAP
}

void loop() {
  for(int color_i=0; color_i<6; color_i++){
    for(int row=0; row<8; row++){
      for(int col=0; col<8; col++){
	setPixel(row, col, primary_colors[color_i]);
	FastLED.show();
      }
    }
  }
}
