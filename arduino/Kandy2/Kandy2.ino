#include <Wire.h>
#include "font8x16.h"
//#include "fatty7x16.h"
#include "Adafruit_DotStar.h"
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

const TimeSpan HOUR(3600);
const TimeSpan MINUTE(60);
const TimeSpan SECOND(1);

TimeSpan countdown_duration(0);

const uint16_t NUMPIXELS = N_ROW * N_COL * N_CLOCK;

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

#define DATAPIN    23
#define CLOCKPIN   24
Adafruit_DotStar strip = Adafruit_DotStar(
    NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

const uint8_t MAX_BRIGHTNESS = 40;
uint32_t color;
uint8_t brightness = 1;
const uint8_t N_MODE = 3;
const uint8_t CLOCK_MODE = 1;
const uint8_t STANDBY_MODE = 2;
const uint8_t RACE_MODE = 3;
uint8_t mode = CLOCK_MODE;

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


void setPixel(byte row, byte col, uint32_t color){
  
  if(false){// flip display?
    row = 8 * N_8x8_ROW - 1 - row;
    col = 8 * N_8x8_COL - 1 - col;
  }
  uint16_t pos = snake(row, col);
  strip.setPixelColor(pos, color);
  strip.setPixelColor(pos + N_8x8_COL*N_8x8_ROW*64, color);
}

uint32_t getPixel(int16_t row, int16_t col){
  uint32_t out = 0;
  if(col >= N_COL){
    int ii = (col - N_COL) * N_ROW + row;
  }
  else{
    out = strip.getPixelColor(snake(row, col));
  }
  return out;
}

void displayChar(uint16_t row, uint16_t col, byte ascii, uint32_t color){
  byte *data = font + ascii * FONT_N_ROW;
  for(uint8_t r=0; r<FONT_N_ROW; r++){
    for(uint8_t c=0; c<FONT_N_COL; c++){
      if((data[r] >> (FONT_N_COL - 1 - c)) & 1){
	setPixel(row + r, col + c, color);
      }
      else{
	setPixel(row + r, col + c, 0);
      }
    }
  }
}

void shiftLeft(uint32_t *col){
  for(uint16_t col=0; col < N_COL - 1; col++){
    for(uint16_t row=0; row < N_ROW; row++){
      setPixel(row, col, getPixel(row, col+1));
    }
  }
  for(uint16_t row=0; row < N_ROW - 1; row++){
    setPixel(row, N_COL - 1, col[row]);
  }
}

void fill(uint8_t r, uint8_t g, uint8_t b) {
  for(uint16_t i=0;i<NUMPIXELS;i++){
    strip.setPixelColor(i, r, g, b);
  }
}

void setup() {
  Serial.begin(115200);
  strip.begin(); // Initialize pins for output
  fill(0, 0, 0);
  strip.setBrightness(255);
  strip.show(); // Turn all LEDs off ASAP
  delay(100);
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
  /*
  */
  stopwatch_start_time = rtc.now().unixtime();
  write_stopwatch_start_time(stopwatch_start_time);
  color = strip.Color(brightness, brightness, brightness); // 40, 40, 40 max for OmniCharge battery on barrel jack + 3A USB
}

void littleDigit(byte d, uint32_t color){
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

void bigDigit(byte start, byte d, uint32_t color){
  byte row, col;
  for(col = 0; col < 8; col++){
    for(row = 0; row < 16; row++){
      if((digits8x16[d * 16 + row] >> col) & 1){
	setPixel(row, col + start, color);
      }
      else{
	setPixel(row, col + start, 0);
      }
    }
  }
}

void littleOne(uint32_t color){
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

void littleTwo(uint32_t color){
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

void displayTime(uint8_t hh, uint8_t mm, uint8_t ss, uint32_t color, bool colen){
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

DateTime now;

void loop(){
  now = rtc.now();
  updateDisplay();
  interact();
}

void updateDisplay(){
  uint32_t empty[16];
  uint16_t ii, row, col;
  byte hh, mm, ss;
  bool pending_start = false;
  TimeSpan race_time;
  
  if(mode == CLOCK_MODE){
    color = strip.Color(brightness, brightness, brightness);
  }
  if(mode == RACE_MODE){
    color = strip.Color(brightness, brightness, brightness);
  }
  if(mode == STANDBY_MODE){
    color = strip.Color(brightness, brightness, 0);
  }    
  if(now.unixtime() > stopwatch_start_time){
    race_time = TimeSpan(now.unixtime() - stopwatch_start_time);
  }
  else if(mode == RACE_MODE){
    race_time = TimeSpan(stopwatch_start_time - now.unixtime());
    color = strip.Color(brightness, 0, 0);
    pending_start = true;
  }

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

  if(mode == STANDBY_MODE){
    // race time
    uint8_t race_hh = countdown_duration.hours();
    uint8_t race_mm = countdown_duration.minutes();
    uint8_t race_ss = countdown_duration.seconds();
    displayTime(race_hh % 100, race_mm, race_ss, color, true);
  }
  else if(mode == RACE_MODE){
    // race time
    uint8_t race_hh = race_time.hours() + race_time.days() * 24;
    uint8_t race_mm = race_time.minutes();
    uint8_t race_ss = race_time.seconds();
    displayTime(race_hh % 100, race_mm, race_ss, color, true);
    setPixel(0, 0, color);
    if(pending_start){
      if(race_hh == 0 && race_mm == 0 && race_ss < 10){
	if(race_ss > 0){
	  fill(brightness, 0, 0);
	  for(int col=0; col<10; col++){
	    for(int row=0; row<16; row++){
	      setPixel(row, col+23, 0);
	    }
	  }
	  bigDigit(24, race_ss, strip.Color(brightness, brightness, brightness));
	}
	else{
	  fill(0, brightness, 0);
	}
      }
    }
    else{
      if(race_hh == 0 && race_mm == 0 && race_ss < 5){
	if(race_ss % 2 == 0){
	  fill(0, brightness, 0);
	}
	else{
	  fill(0, 0, 0);
	}
      }
    }
  }
  else{
    // clock is default
    displayTime(hh % 100, mm, ss, color, true);
  }
  strip.show();
  // delay(1000);
  return;
}

void tick(){
  while(millis() % 1000 > 5){
    interact();
  }
}
void interact(){
  uint8_t command;
  
  while(Serial.available()){
    command = Serial.read();
    do_command(command);
    updateDisplay();
  }
}
void do_command(uint8_t command){
  uint8_t hh = now.hour();
  uint8_t mm = now.minute();
  uint8_t ss = now.second();

  switch(command){
  case 1: // start / resume
    if(mode == STANDBY_MODE){
      stopwatch_start_time = (now + countdown_duration).unixtime();
      mode = RACE_MODE;
    }
    break;
  case 2: // stop / pause
    break;
  case 3:
    if(mode == CLOCK_MODE){
      now = now + HOUR;
      rtc.adjust(now);
    }
    break;
  case 4:
    if(mode == CLOCK_MODE){
      now = now - HOUR;
      rtc.adjust(now);
    }
    break;
  case 5:
    if(mode == CLOCK_MODE){
      now = now + MINUTE;
      rtc.adjust(now);
    }
    break;
  case 6:
    if(mode == CLOCK_MODE){
      now = now - MINUTE;
      rtc.adjust(now);
    }
    break;
  case 7:
    if(mode == CLOCK_MODE){
      now = now + SECOND;
      rtc.adjust(now);
    }
    break;
  case 8:
    if(mode == CLOCK_MODE){
      now = now - SECOND;
      rtc.adjust(now);
    }
    break;
  case 9:
    if(mode == CLOCK_MODE){
      now = now - ss;
      rtc.adjust(now);
    }
    break;
  case 10:
    if(mode == STANDBY_MODE){
      countdown_duration = countdown_duration + HOUR;
    }
    break;
  case 11:
    if(mode == STANDBY_MODE){
      if(countdown_duration.hours() > 0){
	countdown_duration = countdown_duration - HOUR;
      }
    }
    break;
  case 12:
    if(mode == STANDBY_MODE){
      countdown_duration = countdown_duration + MINUTE;
    }
    break;
  case 13:
    if(mode == STANDBY_MODE){
      if(countdown_duration.minutes() > 0){
	countdown_duration = countdown_duration - MINUTE;
      }
    }
    break;
  case 14:
    if(mode == STANDBY_MODE){
      countdown_duration = countdown_duration + SECOND;
    }
    break;
  case 15:
    if(mode == STANDBY_MODE){
      if(countdown_duration.seconds() > 0){
	countdown_duration = countdown_duration - SECOND;
      }
    }
    break;
  case 16:
    increment_brightness();
    break;
  case 17:
    decrement_brightness();
    break;
  case 18:
    increment_mode();
    break;
  case 19:
    decrement_mode();
    break;
  case 20:
    now = now - (hh * 3600 + mm * 60 + ss);
    rtc.adjust(now);
    break;
  default:
    break;
  }
}
void increment_brightness(){
  if(brightness < MAX_BRIGHTNESS){
    brightness++;
    color = strip.Color(brightness, brightness, brightness);
  }
}
void decrement_brightness(){
  if(brightness > 1){
    brightness--;
    color = strip.Color(brightness, brightness, brightness);
  }
}

void increment_mode(){
  if(mode < N_MODE){
    mode++;
  }
}
void decrement_mode(){
  if(mode > 1){
    mode--;
  }
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
