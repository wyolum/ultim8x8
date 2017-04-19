#include "font8x16.h"
#include "font4x8.h"
#include "pixel_font.h"
#include "font8x8.h"
#include "font5x8.h"
//#include "fatty7x16.h"
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
//ULTiM8x72
//const byte N_8x8_ROW = 1;
//const byte N_8x8_COL = 9;

//ULTiM16x56
//const byte N_8x8_ROW = 2;
//const byte N_8x8_COL = 7;
//ULTiM16x56
const byte N_8x8_ROW = 4;
const byte N_8x8_COL = 7;

const byte N_ROW = N_8x8_ROW * 8;
const byte N_COL = N_8x8_COL * 8;
const byte BUFFER_SIZE = N_ROW * 8;
const uint16_t NUMPIXELS = N_ROW * N_COL;

// command types
const byte BP_SETUP_DATA = 1;
const byte BP_PIXEL_DATA = 2;
const byte BP_BRIGHTNESS = 3;


// return codes
const byte BP_SUCCESS = 255;
const byte BP_REBOOT = 42;
const byte BP_ERROR = 0;
const byte BP_ERROR_SIZE = 1;
const byte BP_ERROR_UNSUPPORTED = 2;
const byte BP_ERROR_PIXEL_COUNT = 3;

uint8_t brightness = 4;


void show();

CRGB leds[NUMPIXELS];

bool interact(){
  byte cmd;
  uint16_t count, i;
  byte r, g, b;
  bool out = false;
  
  if(Serial.available() >= 3){
    cmd = Serial.read();
    count = ((uint16_t)Serial.read()) * 256 + (uint16_t)Serial.read() - 3;
    if(cmd == BP_SETUP_DATA){ // ignore for now
      for(int ii=0; ii<count; ii++){
	Serial.read();
      }
      Serial.write(BP_SUCCESS); Serial.flush(); delay(1);
    }
    else if(cmd == BP_BRIGHTNESS){
      brightness = Serial.read();
      Serial.write(BP_SUCCESS); Serial.flush(); delay(1);
      FastLED.setBrightness(brightness);
    }
    else if(cmd == BP_PIXEL_DATA && Serial.available() >= 2){
      if(count/3 > NUMPIXELS){
	Serial.write(BP_ERROR_PIXEL_COUNT); Serial.flush(); delay(1);
      }
      else{
	for(i=0; i < count / 3 && Serial.available(); i++){
	  if(Serial.available()){
	    r = Serial.read();
	  }
	  if(Serial.available()){
	    g = Serial.read();
	  }
	  if(Serial.available()){
	    b = Serial.read();
	  }
	  leds[i] = CRGB(r, g, b);
	}
	if(i * 3 == count){
	  Serial.write(BP_SUCCESS); Serial.flush(); delay(1);
	}
	else{
	  Serial.write((char)BP_ERROR_SIZE); Serial.flush(); delay(1);
	}
      }
    }
    else{
      //while(Serial.available()){
	//Serial.read();
      //}
      //Serial.write((char)BP_ERROR_UNSUPPORTED); Serial.flush(); delay(1);
    }
  }
  return out;
}

void show(){
  FastLED.show();
}

void fill(const struct CRGB & color){
  for(int ii=0; ii<NUMPIXELS; ii++){
    leds[ii] = color;
    if(ii % 8 == 7){
      show();
    }
  }
}

void setup() {
  Serial.begin(115200);
  FastLED.setBrightness(brightness);
  FastLED.addLeds<APA102, SCK, MOSI, BGR, DATA_RATE_MHZ(25)>(leds, NUMPIXELS);
  /*
    fill(CRGB::Blue);
    show(); // Turn all LEDs off ASAP
    delay(1000);
    fill(CRGB::Black);
  */
  show(); // Turn all LEDs off ASAP
}

void loop() {
  interact();
  show();
}
