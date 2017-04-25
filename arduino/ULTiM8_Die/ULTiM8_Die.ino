/*
 * A die (one of a pair of dice) simulation for 6 ULTiM8x8's arranged in a cube.
 */
#include "font8x16.h"
#include "font4x8.h"
#include "pixel_font.h"
#include "font8x8.h"
#include "font5x8.h"
//#include "fatty7x16.h"
#include <FastLED.h>
#include <SPI.h>

CRGB primary_colors[6] = {
  CRGB(0, 255, 255),
  CRGB(0, 255, 0),
  CRGB(0, 0, 255),
  CRGB(255, 255, 0),
  CRGB(255, 0, 0),
  CRGB(255, 0, 255),
};


uint8_t   one[8] = {0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00}; // 1
uint8_t   two[8] = {0x00,0x06,0x06,0x00,0x00,0x60,0x60,0x00}; // 2
uint8_t three[8] = {0x03,0x03,0x00,0x18,0x18,0x00,0xc0,0xc0}; // 3
uint8_t  four[8] = {0x00,0x66,0x66,0x00,0x00,0x66,0x66,0x00}; // 5
uint8_t  five[8] = {0xc3,0xc3,0x00,0x18,0x18,0x00,0xc3,0xc3}; // 5
uint8_t   six[8] = {0xc3,0xc3,0x00,0xc3,0xc3,0x00,0xc3,0xc3}; // 6

uint8_t *numbers[6] = {one, two, three, four, five, six};

void draw_face(uint16_t col, uint8_t *number, const struct CRGB & color){
  for(byte ii=0; ii<8; ii++){
    for(byte jj=0; jj<8; jj++){
      if(number[ii] >> jj & 0b1){
	setPixel(ii, (col + jj), color);
      }
      else{
	setPixel(ii, (col + jj), CRGB::Black);
      }
    }
  }
}

const byte N_8x8_ROW = 1;
const byte N_8x8_COL = 6;
const byte N_ROW = N_8x8_ROW * 8;
const byte N_COL = N_8x8_COL * 8;
const byte BUFFER_SIZE = N_ROW * 8;
const uint16_t NUMPIXELS = N_ROW * N_COL + 1;
const byte PIXEL_DATA_CMD = 2;
const byte PIXEL_DATA_SUCCESS = 255;
const byte PIXEL_DATA_ERROR_SIZE = 1;

void show();
void display_count(unsigned int v, const struct CRGB & color){
  bigDigit( 0, (int)(v / 1e6) % 10, color);
  bigDigit( 6, (int)(v / 1e5) % 10, color);
  bigDigit(12, (int)(v / 1e4) % 10, color);
  bigDigit(18, (int)(v / 1e3) % 10, color);
  bigDigit(24, (int)(v / 1e2) % 10, color);
  bigDigit(32, (int)(v / 1e1) % 10, color);
  bigDigit(40, (int)(v / 1e0) % 10, color);
}


CRGB leds[NUMPIXELS];
byte shift_delay_ms = 1;    // global var to make scrolling consistent
int32_t last_serial_ms = -1;
const uint32_t SERIAL_TIMEOUT = 5000;


void show(){
  FastLED.show();
}

const byte digits4x7[4*10] = {
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

const byte digits5x8[8*10] = {
  0x0e,0x11,0x11,0x11,0x11,0x11,0x11,0x0e, // 0
  0x04,0x06,0x04,0x04,0x04,0x04,0x04,0x0e, // 1
  0x0e,0x11,0x10,0x10,0x08,0x04,0x02,0x1f, // 2
  0x0e,0x11,0x10,0x08,0x10,0x10,0x11,0x0e, // 3
  0x08,0x09,0x09,0x09,0x1f,0x08,0x08,0x08, // 4
  0x1f,0x01,0x01,0x0f,0x10,0x10,0x11,0x0e, // 5
  0x0e,0x11,0x01,0x0f,0x11,0x11,0x11,0x0e, // 6
  0x1f,0x11,0x10,0x08,0x04,0x02,0x02,0x02, // 7
  0x0e,0x11,0x11,0x0e,0x11,0x11,0x11,0x0e, // 8
  0x0e,0x11,0x11,0x11,0x1e,0x10,0x11,0x0e, // 9
};

void bigDigit(byte start, byte d, const struct CRGB & color){
  byte row, col;
  for(col = 0; col < 5; col++){
    for(row = 0; row < 8; row++){
      if((digits5x8[d * 8 + row] >> col) & 1){
	setPixel(row, col + start, color);
      }
      else{
	setPixel(row, col + start, 0);
      }
    }
  }
}

bool flip = false;
int32_t snake(byte row, byte col){
  //  1. find the board
  uint32_t out = 0;
  uint8_t board;
  if(flip){
    row = N_ROW - row - 1;
    col = N_COL - col - 1;
  }
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
    out = -1;
  }
  return out;
}

const struct CRGB Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return CRGB(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return CRGB(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return CRGB(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void setPixel(uint8_t row, uint8_t col, const struct CRGB & color){
  if(row >= N_ROW){
  }
  else if(col >= N_COL){
    //int ii = (col - N_COL) * N_ROW + row;
    col = (col - N_COL);
    if(col < 8 && row < N_ROW){
    }
  }
  else{
    uint16_t pos = snake(row, col);
    if(pos != -1){
      leds[pos] = color;
    }
  }
}

PixelFont Font4x8 = PixelFont(4, 8, 8, font4x8, setPixel);
PixelFont Font5x8 = PixelFont(5, 8, 8, font5x8, setPixel);
PixelFont Font8x8 = PixelFont(8, 8, 8, font8x8, setPixel);

CRGB getPixel(int16_t row, int16_t col){
  CRGB out = CRGB::Black;
  if(row >= N_ROW){
  }
  else if(col >= N_COL){
    //int ii = (col - N_COL) * N_ROW + row;
    col = col - N_COL;
    //if(ii < BUFFER_SIZE){
    if(col < 8 && row < N_ROW){
    }
  }
  else{
    out = leds[snake(row, col)];
  }
  return out;
}

void shiftLeft(uint8_t start_row, uint8_t n_row){
  for(uint16_t col=0; col < N_COL; col++){
    for(uint16_t row=start_row; row < start_row + n_row; row++){
      setPixel(row, col, getPixel(row, (col+1) % N_COL));
    }
  }
  for(uint16_t row=start_row; row < start_row + n_row; row++){
    setPixel(row, N_COL + 7, CRGB::Black);
  }
  if(shift_delay_ms){
    delay(shift_delay_ms);
  }
}

void fill(const struct CRGB & color) {
  for(uint16_t i=0;i<NUMPIXELS;i++){
    leds[i] = color;
  }
}

uint8_t brightness = 10;

void setup() {
  Serial.begin(115200);
  for(int ii=0; ii<BUFFER_SIZE; ii++){
  }
  FastLED.setBrightness(brightness);
  FastLED.addLeds<APA102, SCK, MOSI, BGR, DATA_RATE_MHZ(25)>(leds, NUMPIXELS);
  show(); // Turn all LEDs off ASAP
  delay(1000);
}

void displayString(char *msg, uint8_t row, uint8_t col,
		   PixelFont font, const struct CRGB & color, const struct CRGB & background){
  uint16_t ii;
  for(ii=0; ii<min(strlen(msg), N_COL / font.width - col + 1); ii++){
    font.drawChar(msg[ii], row, col + ii * font.width, color, background);
  }
}

void scrollChar(byte c, uint8_t row, PixelFont font,
		const struct CRGB & color, const struct CRGB & background){
  font.drawChar(c, row, N_COL - font.width * 0, color, background);
  for(int ii=0; ii < font.width; ii++){
    //shiftLeft(0, 16);
    shiftLeft(row, font.height);
    show();
  }
}

void scrollMsg(char *msg, uint8_t row, PixelFont font,
	       const struct CRGB & color, const struct CRGB & background){
  for(int ii=0; ii < strlen(msg); ii++){
    scrollChar(msg[ii], row, font, color, background);
  }
}
	       
void littleDigit(byte d, const struct CRGB & color){
  byte row, col;
  for(col = 0; col < 4; col++){
    for(row = 0; row < 7; row++){
      if((digits4x7[d * 4 + col] >> row) & 1){
	setPixel(row, col, color);
      }
      else{
	setPixel(row, col, 0);
      }
    }
  }
}

void littleOne(const struct CRGB & color){
  uint8_t ii;
  for(ii=2; ii<12; ii++){
    setPixel(ii, 1, color);
    setPixel(ii, 2, color);
  }
  //setPixel(2, 2, color);
  setPixel(3, 0, color);
  setPixel(11, 0, color);
  setPixel(11, 3, color);
}

void littleTwo(const struct CRGB & color){
  setPixel(1, 0, color);
  setPixel(0, 1, color);
  setPixel(0, 2, color);
  setPixel(1, 3, color);
  setPixel(2, 3, color);
  setPixel(3, 2, color);
  setPixel(4, 1, color);
  setPixel(5, 0, color);
  setPixel(6, 0, color);
  setPixel(6, 1, color);
  setPixel(6, 2, color);
  setPixel(6, 3, color);
}

void displayTime(uint8_t hh, uint8_t mm, uint8_t ss, const struct CRGB & color, bool colen){
  fill(CRGB::Black);

  if(hh>10){
    Font5x8.drawChar('0' + ((hh / 10) % 10), 0, 0, color, CRGB::Black);
  }
  else{
    Font5x8.drawChar(' ' + ((hh / 10) % 10), 0, 0, color, CRGB::Black);
  }
  Font5x8.drawChar('0' + (hh % 10), 0, 5, color, CRGB::Black);

  Font5x8.drawChar('0' + ((mm / 10) % 10), 0, 13, color, CRGB::Black);
  Font5x8.drawChar('0' + (mm % 10), 0, 18, color, CRGB::Black);

  if(colen){
    setPixel(5, 19, color);
    setPixel(8, 19, color);
  }
}

uint32_t start_time = 12 * 3600 + 06 * 60 + 0;
uint32_t count;
char *msg = "     Your Name In Lights!!!   ";

  int xxx = 0;
void loop() {
  uint8_t roll = 8;
  for(int jj = 0; jj < 40; jj++){
    uint8_t new_roll = random(0, 6);
    while(new_roll == roll){
      new_roll = random(0, 6);
    }
    roll = new_roll;
    for(int ii=0; ii<6; ii++){
      draw_face(ii * 8,
		numbers[(ii + roll) % 6],
		primary_colors[(ii + roll) % 6]);
    }
    show();
    delay(10 + jj * 6);
  }
  delay(3000);
}
