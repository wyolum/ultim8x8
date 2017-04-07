#ifndef pixel_font_h
#define pixel_font_h
#include <Arduino.h>
#include <FastLED.h>

class PixelFont{
 public:
  uint8_t width;
  uint8_t height;
  uint8_t bytes_per_char;
  uint8_t *font;
  void (*setter_fp)(uint8_t row, uint8_t col,
		    const struct CRGB & color);
  
  PixelFont(uint8_t _width, uint8_t _height, uint8_t _bytes_per_char,
	    uint8_t *_font,
	    void (*_setter_fp)(uint8_t row, uint8_t col,
			      const struct CRGB & color));
  void drawChar(uint8_t ascii, uint8_t row, uint8_t col,
		const struct CRGB & (*color_fp)(uint8_t row, uint8_t col),
		const struct CRGB & background);
};

const struct CRGB & get_red(uint8_t row, uint8_t col);
const struct CRGB & get_green(uint8_t row, uint8_t col);
const struct CRGB & get_blue(uint8_t row, uint8_t col);

#endif
