#include "font8x16.h"
#include "font4x8.h"
#include "pixel_font.h"
#include "font8x8.h"
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
const byte N_8x8_COL = 12;
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

#define DATAPIN    MOSI
#define CLOCKPIN   SCK
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

PixelFont big_font = PixelFont(8, 16, 16, font8x16,
			       setPixel);
PixelFont small_font = PixelFont(4, 8, 8, font4x8,
				 setPixel);
PixelFont fat_font = PixelFont(8, 8, 8, font8x8,
			       setPixel);

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
  Serial.println("start setup!");
  for(int ii=0; ii<BUFFER_SIZE; ii++){
    //rightBuffer[ii] = 0;
    //leftBuffer[ii] = 0;
  }
  FastLED.setBrightness(brightness);
  FastLED.addLeds<APA102, MOSI, SCK, BGR, DATA_RATE_MHZ(25)>(leds, NUMPIXELS);
  FastLED.show(); // Turn all LEDs off ASAP
  Serial.println("end setup!");
  //fill(CRGB::White);
}

int min(int x, int y){
  int out;
  
  if(x < y) out = x;
  else out = y;
  return out;
}
void displayString(char *msg, uint8_t row, uint8_t col,
		   PixelFont font, const struct CRGB & color, const struct CRGB & background){
  uint16_t ii;
  for(ii=0; ii<min(strlen(msg), N_COL / font.width - col); ii++){
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
    big_font.drawChar('0' + ((hh / 10) % 10), 0, 3, color, CRGB::Black);
  }
  else{
    big_font.drawChar(' ' + ((hh / 10) % 10), 0, 3, color, CRGB::Black);
  }
  big_font.drawChar('0' + (hh % 10), 0, 3 + 8, color, CRGB::Black);

  big_font.drawChar('0' + ((mm / 10) % 10), 0, 21, color, CRGB::Black);
  big_font.drawChar('0' + (mm % 10), 0, 21 + 8, color, CRGB::Black);

  big_font.drawChar('0' + ((ss / 10) % 10), 0, 39, color, CRGB::Black);
  big_font.drawChar('0' + (ss % 10), 0, 39 + 8, color, CRGB::Black);
  if(colen){
    setPixel(5, 19, color);
    setPixel(8, 19, color);
    setPixel(5, 19+18, color);
    setPixel(8, 19+18, color);
  }
}

int count = 9 * 3600 + 59*60 + 50;
char *msg = "     Your Name In Lights!!!   ";

  int xxx = 0;
void loop() {
  byte hh, mm, ss;
  const struct CRGB & color = CRGB::White;
  
  fill(CRGB::Black);
  char *top    = "  Hello       ";
  char *bottom = "  World!      ";
  for(int ii=0; ii < min(strlen(top), strlen(bottom)); ii++){
    small_font.drawChar(top[ii],                     0, N_COL, Wheel(      2 * ii), CRGB::Black);
    small_font.drawChar(bottom[ii], small_font.height, N_COL, Wheel(128 + 2 * ii), CRGB::Black);
    FastLED.show();
    for(int ii = 0; ii<small_font.width; ii++){
      shiftLeft(0, 16);
      FastLED.show();
      delay(100);
    }
  }
  delay(1000);
  for(int ii=65; ii<65+26; ii+=2){
    small_font.drawChar(ii,                     0, N_COL, Wheel(      2 * ii), CRGB::Black);
    small_font.drawChar(ii + 1, small_font.height, N_COL, Wheel(128 + 2 * ii), CRGB::Black);
    FastLED.show();
    for(int ii = 0; ii<4; ii++){
      shiftLeft(0, 16);
      FastLED.show();
      delay(100);
    }
  }
  delay(1000);

  hh = (count / 3600);
  mm = (count - hh * 3600) / 60;
  ss = count % 60;
  count += 1;

  displayTime(hh, mm, ss, color, true);
  FastLED.show();
  delay(1000);
  fill(CRGB::Black);

  displayString("Hello", 0, 8, big_font, color, CRGB::Black);
  FastLED.show();
  delay(1000);
  displayString("World", 0, 8, big_font, color, CRGB::Black);
  FastLED.show();
  delay(1000);
  fill(CRGB::Black);
  displayString("Hello", 0, 8, fat_font, color, CRGB::Black);
  FastLED.show();
  delay(1000);
  displayString("World!!", 8, 8, fat_font, color, CRGB::Black);
  FastLED.show();
  delay(1000);
  scrollMsg(" Hello World!!!        ", 0, big_font, CRGB::Blue, CRGB::Black);
  displayString("9:59 AM", 8, 0, fat_font, CRGB::White, CRGB::Black);
  scrollMsg(" Hello World!!!          ", 0, fat_font, CRGB::Blue, CRGB::Black);
  scrollMsg(" Hello ", 0, fat_font, CRGB::Blue, CRGB::Black);
  scrollMsg(" World!", fat_font.height, fat_font, CRGB::Blue, CRGB::Black);
  delay(1000);
  scrollMsg("     Hello    ", 0, small_font, CRGB::Blue, CRGB::Black);
  scrollMsg("     World!!! ", small_font.height, small_font, CRGB::Blue, CRGB::Black);
  delay(1000);
  scrollMsg("          ", 0, big_font, CRGB::Blue, CRGB::Black);
  return;
  for(char ii='A'; ii<='Z'; ii++){
    scrollChar(ii, 0, big_font, Wheel((ii * 10) %256), CRGB::Black);
  }
  for(char ii='a'; ii<='z'; ii++){
    scrollChar(ii, 0, big_font, Wheel((ii * 10) %256), CRGB::Black);
  }
  scrollMsg("          ", 0, big_font, CRGB::Blue, CRGB::Black);
  /*
  */
  for(int ii=0; ii<8; ii++){
    big_font.drawChar('0' + ii, 0,  ii * 8, primary_colors[ii%6], CRGB::Black);
    FastLED.show();
    delay(100);
  }
  for(int ii=0; ii<6; ii++){
    small_font.drawChar('0' + ii, 0,  32 + ii * 4,
			primary_colors[(ii + 1)%6], CRGB::Black);
    FastLED.show();
    delay(100);
    small_font.drawChar('0' + ii + 10, 8,  32 + ii * 4,
			primary_colors[ii%6], CRGB::Black);
    FastLED.show();
    delay(100);
  }
  fat_font.drawChar('J', 0,  0, primary_colors[0],  CRGB::Black);
  FastLED.show();
  delay(100);
  fat_font.drawChar('S', 8,  0, primary_colors[1], CRGB::Black);
  FastLED.show();
  delay(1000);
  fill(CRGB::Black);

  count++;
}
