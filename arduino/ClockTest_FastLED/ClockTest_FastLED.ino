#include <Wire.h>
#include "font8x16.h"
#include <FastLED.h>
#include <colorutils.h>
#include <SPI.h>
#include "RTClib.h"

RTC_DS3231 rtc;

const int DS3231_ADDR = 104;
const int DS3231_ALARM1_OFSET = 0x7;
const byte N_8x8_ROW = 2;
const byte N_8x8_COL = 7;
const byte N_CLOCK = 2;
const byte N_ROW = N_8x8_ROW * 8;
const byte N_COL = N_8x8_COL * 8;
const byte BUFFER_SIZE = N_ROW * 8;

const uint16_t NUMPIXELS = N_ROW * N_COL * N_CLOCK;
const int FRAMES_PER_SECOND = 20;
CRGB leds[NUMPIXELS];
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t brightness = 3;
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))


const byte digits4x7[4 * 10] = {
  62,  65,  65,  62, // 0
   0,  66, 127,  64, // 1
  98,  81,  73,  70, // 2
  34,  73,  73,  54, // 3
  30,  16,  16, 127, // 4
  39,  69,  69,  57, // 5
  62,  73,  73,  50, // 6
  97,  17,   9,   7, // 7
  54,  73,  73,  54, // 8
  38,  73,  73,  62  // 9
};

const byte digits8x16[10 * 16] = {
  0x3c,0x7e,0xe7,0xc3,0xc3,0xc3,0xc3,0xc3,0xc3,0xc3,0xc3,0xc3,0xc3,0xe7,0x7e,0x3c, // 0
  0x18,0x1c,0x1e,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x7e,0x7e, // 1
  0x3c,0x7e,0xe7,0xc3,0xc0,0xc0,0xe0,0x70,0x38,0x1c,0x0e,0x07,0x03,0x03,0xff,0xff, // 2
  0x3c,0x7e,0xe7,0xc3,0xc0,0xc0,0xe0,0x78,0x78,0xe0,0xc0,0xc0,0xc3,0xe7,0x7e,0x3c, // 3
  0x60,0x63,0x63,0x63,0x63,0x63,0x63,0xff,0xff,0x60,0x60,0x60,0x60,0x60,0x60,0x60, // 4
  0xff,0xff,0x03,0x03,0x03,0x03,0x3f,0x7f,0xe0,0xc0,0xc0,0xc0,0xc3,0xe7,0x7e,0x3c, // 5
  0x3c,0x7e,0xe7,0xc3,0x03,0x03,0x03,0x3f,0x7f,0xe3,0xc3,0xc3,0xc3,0xe7,0x7e,0x3c, // 6
  0xff,0xff,0xc0,0xc0,0xe0,0x70,0x38,0x1c,0x0e,0x06,0x06,0x06,0x06,0x06,0x06,0x06, // 7
  0x3c,0x7e,0xe7,0xc3,0xc3,0xc3,0xe7,0x7e,0x7e,0xe7,0xc3,0xc3,0xc3,0xe7,0x7e,0x3c, // 8
  0x3c,0x7e,0xe7,0xc3,0xc3,0xc3,0xc7,0xfe,0xfc,0xc0,0xc0,0xc0,0xc3,0xe7,0x7e,0x3c, // 9
};
uint32_t stopwatch_start_time = 0;

uint32_t snake(byte row, byte col){
  //  1. find the board
  uint32_t out = 0;
  uint8_t board;
  if (row / 8 % 2 == 0){
    board = col / 8; // # assume 2 rows of boards
    if(col % 2 ==  0){
      out = 7 - row + col * 8;
    }
    else{
      out = col * 8 + row;
    }
  }
  else{
    board = 7 - (col - 8 * N_8x8_COL) / 8 - 1; //# assume 2 rows of boards
    //col = 47 - col;
    col = N_8x8_COL * 8 - 1 - col;
    row -= 8;
    if(col % 2 == 0){
      out = 64 * N_8x8_COL + row + col * 8;
    }
    else{
      out = 64 * N_8x8_COL + (7 - row) + col * 8;
    }
  }

  if(out > NUMPIXELS){
    out = 0;
  }
  /*
  Serial.print(row);
  Serial.print(" ");
  Serial.print(col);
  Serial.print(" ");
  Serial.print(board);
  Serial.print(" ");
  Serial.println(out);
  */
  return out;
}


void setPixel(byte row, byte col, const struct CRGB & color){
  
  if(false){// flip display?
    row = 8 * N_8x8_ROW - 1 - row;
    col = 8 * N_8x8_COL - 1 - col;
  }
  uint16_t pos = snake(row, col);
  leds[pos] = color;
  leds[pos + N_8x8_COL*N_8x8_ROW*64] = color;
}

void displayChar(uint16_t row, uint16_t col, byte ascii, const struct CRGB & color){
  byte *data = font + ascii * FONT_N_ROW;
  for(uint8_t r=0; r<FONT_N_ROW; r++){
    for(uint8_t c=0; c<FONT_N_COL; c++){
      if((data[r] >> (FONT_N_COL - 1 - c)) & 1){
	setPixel(row + r, col + c, color);
      }
      else{
	setPixel(row + r, col + c, CRGB::Black);
      }
    }
  }
}

void fill(uint8_t r, uint8_t g, uint8_t b) {
  for(uint16_t i=0;i<NUMPIXELS;i++){
    leds[i] = 0;
  }
}

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { confetti };//, sinelon, juggle, bpm, rainbow, rainbowWithGlitter};

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUMPIXELS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUMPIXELS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUMPIXELS, 10);
  int pos = random16(NUMPIXELS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUMPIXELS, 20);
  int pos = beatsin16(13,0,NUMPIXELS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUMPIXELS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUMPIXELS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NUMPIXELS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}


void setup() {
  delay(3000); // 3 second delay for recovery
  Wire.begin();
  
  // tell FastLED about the LED strip configuration
  // FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<APA102>(leds, NUMPIXELS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(brightness);
}

void littleDigit(byte d, const struct CRGB & color){
  byte row, col;
  for(col = 0; col < 4; col++){
    for(row = 0; row < 7; row++){
      if((digits4x7[d * 4 + col] >> row) & 1){
	setPixel(row, col, color);
      }
      else{
	setPixel(row, col, CRGB::Black);
      }
    }
  }
}

void bigDigit(byte start, byte d, const struct CRGB & color){
  byte row, col;
  for(col = 0; col < 8; col++){
    for(row = 0; row < 16; row++){
      if((digits8x16[d * 16 + row] >> col) & 1){
	setPixel(row, col + start, color);
      }
      else{
	setPixel(row, col + start, CRGB::Black);
      }
    }
  }
}

void displayTime(uint8_t hh, uint8_t mm, uint8_t ss, const struct CRGB & color, bool colen){
  char time[7];
  uint8_t ii;
  time[0] = '0' + hh / 10;
  time[1] = '0' + hh % 10;
  time[2] = '0' + mm / 10;
  time[3] = '0' + mm % 10;
  time[4] = '0' + ss / 10;
  time[5] = '0' + ss % 10;
  time[6] = 0;

  if(hh / 10 > 0){
    bigDigit( 0 - 1 + 0, hh / 10, color);
  }
  bigDigit( 9 - 1 + 0, hh % 10, color);
  bigDigit(18 + 1, mm / 10, color);
  bigDigit(27 + 1, mm % 10, color);
  bigDigit(37 + 2, ss / 10, color);
  bigDigit(46 + 2, ss % 10, color);
  if(colen){
    setPixel( 4, 17, color);
    setPixel( 5, 17, color);
    setPixel(10, 17, color);
    setPixel(11, 17, color);

    setPixel( 4, 17 + 20, color);
    setPixel( 5, 17 + 20, color);
    setPixel(10, 17 + 20, color);
    setPixel(11, 17 + 20, color);
  }
  
  /*
    if(ss % 10 < 5){
    bigDigit(0, 0, color);
    bigDigit(9, 1, color);
    bigDigit(18, 2, color);
    bigDigit(27, 3, color);
    bigDigit(36, 4, color);
  }
  else{
    bigDigit(0, 5, color);
    bigDigit(9, 6, color);
    bigDigit(18, 7, color);
    bigDigit(27, 8, color);
    bigDigit(36, 9, color);
  }
  */

  /*
  displayChar(0, 5, time[1], color);
 
  displayChar(0, 15, time[2], color);
  displayChar(0, 15 + 8, time[3], color);

  displayChar(0, 33, time[4], color);
  displayChar(0, 33 + 8, time[5], color);
  if(colen){
    setPixel(5, 13, color);
    setPixel(8, 13, color);
    setPixel(5, 31, color);
    setPixel(8, 31, color);
  }
  else{
    setPixel(5, 13, 0);
    setPixel(8, 13, 0);
    setPixel(5, 31, 0);
    setPixel(8, 31, 0);
  }
  if(10 <= hh && hh < 20){
    littleOne(color);
  }
  if(20 <= hh && hh < 100){
    littleDigit(hh / 10, color);
  }
  */
}

int count = 0;
char *msg = "2x6 ULTIM8x8 array!!!   ";


void loop()
{
  DateTime now = rtc.now();
  
  displayTime(now.hour(), now.minute(), now.second(), CRGB::White, true);
  FastLED.show();  
  FastLED.delay(1000); 
  return;
  // return;
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
}

void loop1() {

  uint32_t empty[16];
  uint16_t ii, row, col;
  byte hh, mm, ss;
  uint32_t color = CRGB::White;

  DateTime now = rtc.now();
  TimeSpan race_time = TimeSpan(rtc.now().unixtime() - stopwatch_start_time);

  fill(0, 0, 0);
  count += 1;
  /*  */

  // wall clock time
  hh = now.hour();
  if(true){ // use 12 hour time
    hh = hh % 12;
    if(hh == 0){
      hh = 12;
    }
  }
  mm = now.minute();
  ss = now.second();
  // race time
  /*
  hh = race_time.hours() + race_time.days() * 24;
  mm = race_time.minutes();
  ss = race_time.seconds();
  */
  displayTime(hh % 100, mm, ss, color, true);
  FastLED.show();
  // delay(1000);
}

uint8_t dec2bcd(int dec){
  uint8_t t = dec / 10;
  uint8_t o = dec - t * 10;
  return (t << 4) + o;
}

void write_stopwatch_start_time(time_t start_time){
  uint8_t *time_bytes_p;
  /*
    set to:
        0 in clock mode
        start time in race mode
  */
  
  time_bytes_p = (uint8_t*)(&start_time);  
  rtc_raw_write(DS3231_ALARM1_OFSET, 4, false, time_bytes_p);
}

time_t read_stopwatch_start_time(){
  uint8_t *time_bytes_p;
  time_t out;
  /*
    set to:
        0 in clock mode
        start time in race mode
  */
  
  time_bytes_p = (uint8_t*)(&out);  
  rtc_raw_read(DS3231_ALARM1_OFSET, 4, false, time_bytes_p);
  return out;
}

int bcd2dec(uint8_t bcd){
  return (((bcd & 0b11110000)>>4)*10 + (bcd & 0b00001111));
}

bool rtc_raw_read(uint8_t addr,
		  uint8_t n_bytes,
		  bool is_bcd,
		  uint8_t *dest){

  bool out = false;
  Wire.beginTransmission(DS3231_ADDR); 
  // Wire.send(addr); 
  Wire.write((uint8_t)(addr));
  Wire.endTransmission();
  Wire.requestFrom(DS3231_ADDR, (int)n_bytes); // request n_bytes bytes 
  if(Wire.available()){
    for(uint8_t i = 0; i < n_bytes; i++){
      dest[i] = Wire.read();
      if(is_bcd){ // needs to be converted to dec
	dest[i] = bcd2dec(dest[i]);
      }
    }
    out = true;
  }
  return out;
}

void rtc_raw_write(uint8_t addr,
		   uint8_t n_byte,
		   bool is_bcd,
		   uint8_t *source){
  uint8_t byte;

  Wire.beginTransmission(DS3231_ADDR);
  Wire.write((uint8_t)(addr));
  for(uint8_t i = 0; i < n_byte; i++){
    if(is_bcd){
      byte = dec2bcd(source[i]);
    }
    else{
      byte = source[i];
    }
    Wire.write(byte);
  }
  Wire.endTransmission();  
}
