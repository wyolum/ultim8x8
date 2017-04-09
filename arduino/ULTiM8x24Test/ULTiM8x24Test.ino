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
const byte N_8x8_ROW = 1;
const byte N_8x8_COL = 3;
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


CRGB rightBuffer[N_ROW][8];
//CRGB rightBuffer[BUFFER_SIZE];
CRGB leftBuffer[BUFFER_SIZE];

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
      rightBuffer[row][col] = color;
    }
  }
  else{
    uint16_t pos = snake(row, col);
    leds[pos] = color;
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
      out = rightBuffer[row][col];
    }
  }
  else{
    out = leds[snake(row, col)];
  }
  return out;
}

void shiftLeft(uint8_t start_row, uint8_t n_row){
  for(uint16_t col=0; col < N_COL + 7; col++){
    for(uint16_t row=start_row; row < start_row + n_row; row++){
      setPixel(row, col, getPixel(row, col+1));
    }
  }
  for(uint16_t row=start_row; row < start_row + n_row; row++){
    setPixel(row, N_COL + 7, CRGB::Black);
  }
}

void fill(const struct CRGB & color) {
  for(uint16_t i=0;i<NUMPIXELS;i++){
    leds[i] = color;
  }
}

#define SerialDBG SERIAL_PORT_USBVIRTUAL
uint8_t brightness = 4;

void setup() {
  Serial.begin(115200);
  Serial.println("HERE!");
  for(int ii=0; ii<BUFFER_SIZE; ii++){
    //rightBuffer[ii] = 0;
    //leftBuffer[ii] = 0;
  }
  FastLED.setBrightness(brightness);
  FastLED.addLeds<APA102, SCK, MOSI, BGR, DATA_RATE_MHZ(25)>(leds, NUMPIXELS);
  FastLED.show(); // Turn all LEDs off ASAP
  //fill(CRGB::White);
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
    FastLED.show();
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
  byte hh, mm, ss;
  const struct CRGB & color = CRGB::White;

  uint32_t tm = millis() / 1000 + start_time;
  
  hh = (tm / 3600) % 12;
  if(hh == 0){
    hh = 12;
  }
  mm = (tm / 60) % 60;
  ss = (tm / 1) % 60;
  
  fill(CRGB::Black);
  if(hh > 9){
    bigDigit( 0, hh/10, color);
  }
  bigDigit( 5, hh%10, color);
  bigDigit(13, mm/10, color);
  bigDigit(19, mm%10, color);
  setPixel(2, 11, color);
  setPixel(3, 11, color);
  setPixel(5, 11, color);
  setPixel(6, 11, color);
  FastLED.show();
  delay(1000);
  //displayTime(11, 59, ss, color, true);

  //Font5x8.drawChar('0', 0, 0, color, CRGB::Black);

  tm++;
}
