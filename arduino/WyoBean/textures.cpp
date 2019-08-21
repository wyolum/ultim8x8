#include "textures.h"

void rainbow(CRGB *leds, uint16_t current_time, uint16_t (*XY)(uint8_t col, uint8_t row)) {
  int i, dx, dy;
  CHSV hsv;
  float dist;
  
  hsv.hue = 0;
  hsv.val = 255;
  hsv.sat = 240;
  int count = ((current_time % 300) * 255) / 300;
  for( int row = 0; row < 24; row++) {
    for( int col = 0; col < 56; col++) {
      dy = (row - 4) * 2;
      dx = col - 8;
      dist = sqrt(dx * dx + dy * dy);
      i = XY(col, row);
      //hsv.hue =  ((int)(dist * 16) - count) % 256;
      hsv.hue =  (count + (24 * row + col) * 2) % 256;
      leds[i] = hsv;
    }
  }
  // Show the leds (only one of which is set to white, from above)
  //delay(100);
}
