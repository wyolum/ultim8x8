#include "pixel_font.h"

PixelFont::PixelFont(uint8_t _width, uint8_t _height, uint8_t _bytes_per_char,
	    uint8_t *_font,
	    void (*_setter_fp)(uint8_t row, uint8_t col,
			      const struct CRGB & color)){
  width = _width;
  height = _height;
  bytes_per_char = _bytes_per_char;
  font = _font;
  setter_fp = _setter_fp;
}

void PixelFont::drawChar(uint8_t ascii, uint8_t row, uint8_t col,
			 const struct CRGB & color,
			 const struct CRGB & background){
  byte *data = font + ascii * bytes_per_char;
  for(uint8_t r=0; r<height; r++){
    for(uint8_t c=0; c<width; c++){
      if((data[r] >> (8 - 1 - c)) & 1){
	setter_fp(row + r, col + c, color);
      }
      else{
	setter_fp(row + r, col + c, background);
      }
    }
  }
}

void PixelFont::drawChar(uint8_t ascii, uint8_t row, uint8_t col,
			 const struct CRGB & (*color_fp)(uint8_t row, uint8_t col),
			 const struct CRGB & background){
  byte *data = font + ascii * bytes_per_char;
  for(uint8_t r=0; r<height; r++){
    for(uint8_t c=0; c<width; c++){
      if((data[r] >> (8 - 1 - c)) & 1){
	setter_fp(row + r, col + c, color_fp(row + r, col + c));
      }
      else{
	setter_fp(row + r, col + c, background);
      }
    }
  }
}

const struct CRGB & get_red(uint8_t row, uint8_t col){
  static int xxx = 0;
  Serial.println(xxx++);
  return CRGB::Red;
}
const struct CRGB & get_green(uint8_t row, uint8_t col){
  static int xxx = 0;
  Serial.println(xxx++);
  return CRGB::Green;
}
const struct CRGB & get_blue(uint8_t row, uint8_t col){
  static int xxx = 0;
  Serial.println(xxx++);
  return CRGB::Blue;
}
