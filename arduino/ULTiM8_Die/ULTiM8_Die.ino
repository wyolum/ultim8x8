/*
 * A die (one of a pair of dice) simulation for 6 ULTiM8x8's arranged in a cube.
 */
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

void show();

CRGB leds[NUMPIXELS];
byte shift_delay_ms = 1;    // global var to make scrolling consistent
int32_t last_serial_ms = -1;
const uint32_t SERIAL_TIMEOUT = 5000;


void show(){
  FastLED.show();
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

uint32_t start_time = 12 * 3600 + 06 * 60 + 0;
uint32_t count;
char *msg = "     Your Name In Lights!!!   ";

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
