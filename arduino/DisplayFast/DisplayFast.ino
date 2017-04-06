#include "font8x16.h"
#include "font4x8.h"
//#include "font8x8.h"
//#include "fatty7x16.h"
#include <FastLED.h>
#include <SPI.h>

CRGB primary_colors[3] = {
  CRGB::Red,
  CRGB::Green,
  CRGB::Blue
};
const byte N_8x8_ROW = 2;
const byte N_8x8_COL = 7;
const byte N_ROW = N_8x8_ROW * 8;
const byte N_COL = N_8x8_COL * 8;
const byte BUFFER_SIZE = N_ROW * 8;

const uint16_t NUMPIXELS = N_ROW * N_COL;
CRGB leds[NUMPIXELS];

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
  
//uint32_t my_display[NUMPIXELS];

#define DATAPIN    23
#define CLOCKPIN   24
#define LEDVAL 1

int32_t snake(byte row, byte col){
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
    out = -1;
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
uint32_t snake_orig(byte row, byte col){
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
    board = 6 - (col - 8 * N_8x8_COL) / 8 - 1; //# assume 2 rows of boards
    col = 47 - col;
    row -= 8;
    if(col % 2 == 0){
      out = 64 * 6 + row + col * 8;
    }
    else{
      out = 64 * 6 + (7 - row) + col * 8;
    }
  }
  return out;
}


CRGB rightBuffer[BUFFER_SIZE];
CRGB leftBuffer[BUFFER_SIZE];

void setPixel(byte row, byte col, const struct CRGB & color){
  if(col >= N_COL){
    int ii = (col - N_COL) * N_ROW + row;
    if(ii < BUFFER_SIZE){
      rightBuffer[ii] = color;
    }
  }
  else{
    uint16_t pos = snake(row, col);

    leds[pos] = color;
  }
}

CRGB getPixel(int16_t row, int16_t col){
  CRGB out = 0;
  if(col >= N_COL){
    int ii = (col - N_COL) * N_ROW + row;
    if(ii < BUFFER_SIZE){
      out = rightBuffer[ii];
    }
  }
  else{
    out = leds[snake(row, col)];
  }
  return out;
}

void displayChar8x16(uint16_t row, uint16_t col, byte ascii, const struct CRGB & color){
  byte *data = font8x16 + ascii * FONT8x16_N_ROW;
  for(uint8_t r=0; r<FONT8x16_N_ROW; r++){
    for(uint8_t c=0; c<FONT8x16_N_COL; c++){
      if((data[r] >> (FONT8x16_N_COL - 1 - c)) & 1){
	setPixel(row + r, col + c, color);
      }
      else{
	setPixel(row + r, col + c, 0);
      }
    }
  }
}

void displayChar4x8(uint16_t row, uint16_t col, byte ascii, const struct CRGB & color){
  byte *data = font4x8 + ascii * FONT4x8_N_ROW;
  for(uint8_t r=0; r<FONT4x8_N_ROW; r++){
    for(uint8_t c=0; c<FONT4x8_N_COL; c++){
      if((data[r] >> (FONT4x8_N_COL - 1 - c)) & 1){
	setPixel(row + r, col + c, color);
      }
      else{
	setPixel(row + r, col + c, 0);
      }
    }
  }
}

void shiftLeft(CRGB *col, uint8_t start_row, uint8_t n_row){
  for(uint16_t col=0; col < N_COL - 1; col++){
    for(uint16_t row=start_row; row < start_row + n_row; row++){
      setPixel(row, col, getPixel(row, col+1));
    }
  }
  for(uint16_t row=start_row; row < start_row + n_row; row++){
    setPixel(row, N_COL - 1, col[row]);
  }
}

void fill(const struct CRGB & color) {
  for(uint16_t i=0;i<NUMPIXELS;i++){
    leds[i] = color;
  }
}

#define SerialDBG SERIAL_PORT_USBVIRTUAL
uint8_t brightness = 1;

void setup() {
  SerialDBG.begin(9600);
  SerialDBG.println("HERE!!!");
  for(int ii=0; ii<BUFFER_SIZE; ii++){
    rightBuffer[ii] = 0;
    leftBuffer[ii] = 0;
  }
  FastLED.setBrightness(brightness);
  FastLED.addLeds<APA102, SCK, MOSI, BGR, DATA_RATE_MHZ(25)>(leds, NUMPIXELS);
  FastLED.show(); // Turn all LEDs off ASAP
}

void displayString8x16(char *msg, const struct CRGB & color){
  uint16_t ii;
  for(ii=0; ii<min(strlen(msg), 6); ii++){
    displayChar8x16(0, ii * 8, msg[ii], color);
  }
}

void displayString4x8(char *msg, uint8_t row, const struct CRGB & color){
  uint16_t ii;
  for(ii=0; ii<min(strlen(msg), 14); ii++){
    displayChar4x8(row, ii * 4, msg[ii], color);
  }
}

void scrollChar8x16(byte c, const struct CRGB & color){
  uint16_t ii;

  displayChar8x16(0, 8 * N_8x8_COL, c, color);
  for(ii=0; ii < 8; ii++){
    shiftLeft(rightBuffer + ii * N_ROW, 0, 16);
    FastLED.show();
  }
}

void scrollChar4x8(byte c, byte row, const struct CRGB & color){
  uint16_t ii;

  displayChar4x8(row, 56, c, color);
  for(ii=0; ii < 4; ii++){
    shiftLeft(rightBuffer + ii * N_ROW, row, 8);
    FastLED.show();
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
  char time[7];
  uint8_t ii;

  time[0] = '0' + hh / 10;
  time[1] = '0' + hh % 10;
  time[2] = '0' + mm / 10;
  time[3] = '0' + mm % 10;
  time[4] = '0' + ss / 10;
  time[5] = '0' + ss % 10;
  time[6] = 0;

  displayChar8x16(0, 5, time[1], color);
  
  displayChar8x16(0, 15, time[2], color);
  displayChar8x16(0, 15 + 8, time[3], color);

  displayChar8x16(0, 33, time[4], color);
  displayChar8x16(0, 33 + 8, time[5], color);

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
}

int count = 9 * 3600 + 59*60 + 50;
char *msg = "     Your Name In Lights!!!   ";

void loop() {
  uint32_t empty[16];
  uint16_t ii, row, col;
  byte hh, mm, ss;
  const struct CRGB & color = CRGB::White;
  
  fill(CRGB::Black);
  hh = (count / 3600);
  mm = (count - hh * 3600) / 60;
  ss = count % 60;
  count += 1;
  
  displayTime(hh % 100, mm, ss, color, true);
  FastLED.show();
  delay(500);
  //displayTime(hh, mm, ss, color, true);
  //FastLED.show();
  //delay(500);
  //return;
  //displayString4x8("  Your Name  ", 0, color);
  //displayString4x8("In Lights!!!", 8, color);
  //FastLED.show();

  char *top    = "    Your Name  ";
  char *bottom = "    In Lights  ";
  for(int ii=0; ii<strlen(top); ii++){
    //scrollChar4x8(top[ii % strlen(top)], 0, color);
    scrollChar4x8(top[ii % strlen(top)], 0, primary_colors[ii % 3]);
  }
  for(int ii=0; ii<strlen(bottom); ii++){
    //scrollChar4x8(bottom[ii % strlen(bottom)], 8, color);
    scrollChar4x8(bottom[ii % strlen(bottom)], 8, primary_colors[ii % 3]);
  }
  delay(5000);
  count++;
}
