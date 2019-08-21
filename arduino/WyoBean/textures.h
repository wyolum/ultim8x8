#ifndef TEXTURES_H
#define TEXTURES_H

#include <FastLED.h>

void rainbow(CRGB *leds, uint16_t current_time, uint16_t (*XY)(uint8_t col, uint8_t row));
void noop();
void blend_to_rainbow();
void rainbow();
void blend_to_color(CRGB color);
void blend_to_red();
void blend_to_green();
void blend_to_blue();
void fill_red();
void fill_green();
void fill_blue();


#endif
