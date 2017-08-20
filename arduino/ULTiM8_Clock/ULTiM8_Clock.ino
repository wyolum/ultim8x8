#include <Wire.h>
#include "RTClib.h"
#include "font8x16.h"
// #include "font4x8.h"
#include "pixel_font.h"
// #include "font8x8.h"
#include "font5x8.h"
//#include "fatty7x16.h"
#include <FastLED.h>
#include <SPI.h>

RTC_DS3231 rtc;
DateTime now;

const int DS3231_ALARM1_OFFSET = 0x7;
const int DS3231_ALARM2_OFFSET = 0xB;
const int DS3231_ADDR = 104;

const int NORMAL_MODE = 1;
const int SLEEP_MODE = 2;
const int TIME_SET_MODE = 3;
const int ALARM_SET_MODE = 4;
const int DAY_SET_MODE = 5;
const int FIRST_MODE = NORMAL_MODE;
const int LAST_MODE = ALARM_SET_MODE;
  
int mode = NORMAL_MODE;
int32_t wakeup;

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

const int MODE_B = A1;
const int PLUS_B = A2;
const int MINUS_B = A4;
const int N_BUTTON = 3;
const int BUTTONS[N_BUTTON] = {MODE_B, PLUS_B, MINUS_B};

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

void rtc_raw_write(uint8_t addr,
		   uint8_t n_byte,
		   bool is_bcd,
		   uint8_t *source);
bool rtc_raw_read(uint8_t addr,
		  uint8_t n_bytes,
		  bool is_bcd,
		  uint8_t *dest);
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

bool flip = true;
int32_t snake(byte row, byte col){
  //  1. find the board
  uint32_t out = 0;
  uint8_t board;
  if(flip){
    row = N_ROW - row - 1;
    col = N_COL - col - 1;
  }
  if(col % 2 == 0){
    out = col * 8 + (7 - row);
  }
  if(col % 2 == 1){
    out = col * 8 + (row);
  }

  if(out > NUMPIXELS){
    out = -1;
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

// PixelFont Font5x8 = PixelFont(5, 8, 8, font5x8, setPixel);
PixelFont Font5x8 = PixelFont(5, 8, 8, font5x8, setPixel);
// PixelFont Font8x8 = PixelFont(8, 8, 8, font8x8, setPixel);

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
  Serial.println("ULTiM8x8 Rocks!");

  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1){
      for(int i=0; i<3; i++){
	digitalWrite(13, HIGH);
	delay(250);
	digitalWrite(13, LOW);
	delay(250);
      }
      for(int i=0; i<3; i++){
	digitalWrite(13, HIGH);
	delay(500);
	digitalWrite(13, LOW);
	delay(500);
      }
      for(int i=0; i<3; i++){
	digitalWrite(13, HIGH);
	delay(250);
	digitalWrite(13, LOW);
	delay(250);
      }
    }
  }
  
  FastLED.setBrightness(brightness);
  FastLED.addLeds<APA102, SCK, MOSI, BGR, DATA_RATE_MHZ(25)>(leds, NUMPIXELS);
  FastLED.show(); // Turn all LEDs off ASAP

  pinMode(MODE_B, INPUT);
  digitalWrite(MODE_B, HIGH);
  pinMode(PLUS_B, INPUT);
  digitalWrite(PLUS_B, HIGH);
  pinMode(MINUS_B, INPUT);
  digitalWrite(MINUS_B, HIGH);

  wakeup = 4 * 3600 + 55 * 60 + 100 * 86400; 
    
  rtc_raw_write(DS3231_ALARM1_OFFSET, 4, false, (uint8_t*)(&wakeup));
}

void displayString(char *msg, uint8_t row, uint8_t col,
		   PixelFont font, const struct CRGB & color,
		   const struct CRGB & background){
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

void displayTime(uint32_t tm, const struct CRGB & color, bool colen){
  byte hh, mm, ss;

  hh = (tm / 3600) % 24;
  if(hh == 0){
    //hh = 12;
  }
  mm = (tm / 60) % 60;
  ss = (tm / 1) % 60;
  
  if(hh > 9){
    bigDigit( 0, hh/10, color);
  }
  bigDigit( 5, hh%10, color);
  bigDigit(13, mm / 10, color);
  bigDigit(19, mm % 10, color);
  setPixel(2, 11, color);
  setPixel(3, 11, color);
  setPixel(5, 11, color);
  setPixel(6, 11, color);
  FastLED.show();
}

void displayTime(const struct CRGB & color, bool colen){
  uint32_t tm = rtc.now().unixtime();
  displayTime(tm, color, colen);
}

void displayAlarm(const struct CRGB & color, bool colen){
  uint32_t tm = wakeup;
  displayTime(tm, color, colen);
}

void adjust_time(int32_t delta){
  uint32_t tm;
  
  now = rtc.now() + delta;
  rtc.adjust(now);
  fill(CRGB::Black);
  displayTime(CRGB::White, true);
  FastLED.show();
}

void adjust_alarm(int32_t delta){
  uint32_t tm;
  
  wakeup += delta;
  wakeup %= 86400;
  
  fill(CRGB::Black);
  displayAlarm(CRGB::Red, true);
  FastLED.show();
}

// delay for ms or a button state changes
void my_delay(uint32_t ms){
  uint32_t stop_ms = millis() + ms;
  bool button_state[N_BUTTON];
  bool steady = true;
  
  for(int i=0; i<N_BUTTON; i++){
    button_state[i] = digitalRead(BUTTONS[i]);
  }
  while(steady && millis() < stop_ms){
    for(int i=0; i<N_BUTTON; i++){
      if(button_state[i] != digitalRead(BUTTONS[i])){
	steady = false;
      }
    }
  }
}

void interact(){
  int dt = 200;
  bool adjusted = false;
  if(digitalRead(MODE_B) == LOW){
    mode++;
    if(mode > LAST_MODE){
      mode = FIRST_MODE;
    }
    return;
  }
  while(digitalRead(PLUS_B) == LOW){
    adjusted = true;
    if(mode == TIME_SET_MODE){
      adjust_time(60);
      Serial.println("t+60");
    }
    if(mode == ALARM_SET_MODE){
      adjust_alarm(60);
      Serial.println("a+60");
    }
    my_delay(dt);
    if(dt > 10){
      dt -= 10;
    }
  }
  while(digitalRead(MINUS_B) == LOW){
    adjusted = true;
    if(mode == TIME_SET_MODE){
      adjust_time(-60);
      Serial.println("t-60");
    }
    if(mode == ALARM_SET_MODE){
      adjust_alarm(-60);
      Serial.println("a-60");
    }
    my_delay(dt);
    if(dt > 10){
      dt -= 10;
    }
  }
  if(adjusted){
    my_delay(1000);
  }
}

uint32_t start_time = 12 * 3600 + 06 * 60 + 0;
uint32_t count;
char *msg = "     Your Name In Lights!!!   ";

void scroll_up(){
  int row, col;
  
  for(row = 0; row < N_ROW - 1; row++){
    for(col = 0; col < N_COL; col++){
      setPixel(row, col, getPixel(row + 1, col));
    }
  }
}

// delay specified amount of time or until any button is presssed
bool button_delay(int ms){
  uint32_t n = millis();
  while((millis() - n) < ms){
    if(digitalRead(MODE_B) == LOW ||
       digitalRead(PLUS_B) == LOW ||
       digitalRead(MINUS_B) == LOW){
      return true;
    }
  }
  return false;
}

void show(){
  FastLED.show();
  FastLED.show();
}
void sunrise(){
  int row, col;
  CRGB color;

  bool no_button = true;
  int step = 1000;
  
  for(int b = 2; b < 50 && no_button; b+=1){
    FastLED.setBrightness(b);
    color = Wheel(b);
    for(col = 0; col < N_COL / 2 + 1; col++){
      scroll_up();
      setPixel(N_ROW - 1, N_COL / 2 + col, color);
      setPixel(N_ROW - 1, N_COL / 2 - col, color);
      show();
      no_button = !button_delay(step);
    }
  }
  color = CRGB::White;
  for(int b = 50; b < 150 && no_button; b+=1){
    FastLED.setBrightness(b);
    for(col = 0; col < N_COL / 2 + 1; col++){
      scroll_up();
      setPixel(N_ROW - 1, N_COL / 2 + col, color);
      setPixel(N_ROW - 1, N_COL / 2 - col, color);
      show();
      no_button = !button_delay(step);
    }
  }
  while(no_button){
    no_button = !button_delay(100);
  }
  FastLED.setBrightness(brightness);
  fill(CRGB::Black);
  mode = NORMAL_MODE;
}

void loop() {
  uint32_t tm;
  struct CRGB color = CRGB::Black;

  if(mode == NORMAL_MODE){
    color = CRGB::White;
  }
  if(mode == TIME_SET_MODE){
    color = CRGB::Blue;
  }
  if(mode == ALARM_SET_MODE){
    color = CRGB::Red;
  }
  
  interact();
  now = rtc.now();
  fill(CRGB::Black);


  uint32_t time_of_day = now.unixtime() % 86400;
  if(mode == NORMAL_MODE){
    displayTime(color, true);
    show();
  }
  if(mode == SLEEP_MODE){
    fill(CRGB::Black);
    show();
  }
  if(mode == TIME_SET_MODE){
    displayTime(color, true);
    show();
  }
  if(mode == ALARM_SET_MODE){
    displayTime(wakeup, color, true);
    show();
  }
  if(wakeup > 0 && (time_of_day == wakeup % 86400)){
    fill(CRGB::Black);
    sunrise();
  }
  my_delay(500);
}

// RTC raw functions
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

uint8_t dec2bcd(int dec){
  uint8_t t = dec / 10;
  uint8_t o = dec - t * 10;
  return (t << 4) + o;
}
int bcd2dec(uint8_t bcd){
  return (((bcd & 0b11110000)>>4)*10 + (bcd & 0b00001111));
}

