#include <Wire.h>
#include "font8x16.h"
//#include "fatty7x16.h"
#include <FastLED.h>
#include <colorutils.h>
#include <SPI.h>
#include "RTClib.h"
#include "kandy_commands.h"

RTC_DS3231 rtc;
const bool USE_12_HOUR_FORMAT = true;
const int DS3231_ADDR = 104;
const int DS3231_ALARM1_OFFSET = 0x7;
const int DS3231_ALARM2_OFFSET = 0xB;
const byte N_8x8_ROW = 2;
const byte N_8x8_COL = 7;
const byte N_CLOCK = 1;
const byte N_ROW = N_8x8_ROW * 8;
const byte N_COL = N_8x8_COL * 8;
const byte BUFFER_SIZE = N_ROW * 8;

const unsigned long long SECOND = 1;
const unsigned long long MINUTE = 60 * SECOND;
const unsigned long long HOUR = 60 * MINUTE;

TimeSpan countdown_duration(0);

const uint16_t N_PIXEL_PER_CLOCK = N_ROW * N_COL;
const uint16_t N_PIXEL = N_PIXEL_PER_CLOCK * N_CLOCK;
CRGB leds[N_PIXEL];

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

const byte digits4x8[10 * 8] = {
  0x06,0x09,0x09,0x09,0x09,0x09,0x09,0x06, // 0
  0x04,0x06,0x04,0x04,0x04,0x04,0x04,0x0e, // 1
  0x06,0x09,0x08,0x08,0x04,0x02,0x01,0x0f, // 2
  0x06,0x09,0x08,0x04,0x08,0x08,0x09,0x06, // 3
  0x04,0x05,0x05,0x05,0x0f,0x04,0x04,0x04, // 4
  0x0f,0x01,0x01,0x07,0x08,0x08,0x09,0x06, // 5
  0x06,0x09,0x01,0x01,0x07,0x09,0x09,0x06, // 6
  0x0f,0x08,0x08,0x04,0x02,0x01,0x01,0x01, // 7
  0x06,0x09,0x09,0x06,0x09,0x09,0x09,0x06, // 8
  0x06,0x09,0x09,0x0e,0x08,0x08,0x09,0x06, // 9
};
  
//uint8_t brightness = 1;
const uint8_t MAX_BRIGHTNESS = 40;
const uint8_t N_MODE = 4;
const uint8_t CLOCK_MODE = 1;
const uint8_t STANDBY_MODE = 2;
const uint8_t RACE_MODE = 3;
const uint8_t WAVE_MODE = 4;
const uint16_t MIN_WAVE_SEP = 30;

DateTime now;
uint32_t stopwatch_start_time = 0;
uint8_t brightness = 1;
uint8_t mode = CLOCK_MODE;
uint8_t n_wave = 1;
uint16_t wave_sep = 60;
bool racing = false;


const struct CRGB & Wheel(byte WheelPos) {
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

  if(out > N_PIXEL){
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

void setPixel(byte row, byte col, const struct CRGB & color){
  
  if(false){// flip display?
    row = 8 * N_8x8_ROW - 1 - row;
    col = 8 * N_8x8_COL - 1 - col;
  }
  uint16_t pos = snake(row, col);
  if(0 <= pos && pos < N_PIXEL_PER_CLOCK){
    leds[pos] = color;                           // get front side
    if(N_CLOCK > 1 && (pos + N_PIXEL_PER_CLOCK < N_PIXEL)){
      leds[pos + N_PIXEL_PER_CLOCK] =  color; // get flip side
    }
  }
}

void draw_colen(uint8_t col, const struct CRGB & color){
  setPixel( 4, col, color);
  setPixel( 5, col, color);
  setPixel(10, col, color);
  setPixel(11, col, color);
}
void draw_colens(const struct CRGB & color){
  draw_colen(17, color);
  draw_colen(17 + 20, color);
}
void draw_dash(int16_t col, const struct CRGB & color){
  for(int16_t c = col; c < col + 8; c++){
    setPixel(7, c, color);
    setPixel(8, c, color);
  }
}
void draw_dashes(const struct CRGB & color){
  draw_dash(-1, color);
  draw_dash(8, color);

  draw_dash(19, color);
  draw_dash(28, color);

  draw_dash(39, color);
  draw_dash(48, color);
}

const struct CRGB & getPixel(int16_t row, int16_t col){
  uint32_t out = 0;
  int32_t pos;
  if(col < N_COL){
    pos = snake(row, col);
    if(0 <= pos && pos < N_PIXEL){
      out = leds[pos];
    }
  }
  return out;
}

void displayChar(uint16_t row, uint16_t col, byte ascii, const struct CRGB & color){
  byte *data = font8x16 + ascii * FONT8x16_N_ROW;
  for(uint8_t r=0; r<FONT8x16_N_ROW; r++){
    for(uint8_t c=0; c<FONT8x16_N_COL; c++){
      if((data[r] >> (FONT8x16_N_COL - 1 - c)) & 1){
	setPixel(row + r, col + c, color);
      }
      else{
	setPixel(row + r, col + c, CRGB::Black);
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

void fill(const struct CRGB & color) {
  for(uint16_t i = 0;i < N_PIXEL; i++){
    leds[i] = color;
  }
}

void write_stopwatch_start_time(time_t start_time);
void interact();
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Kandy2... setup()");
  Serial1.begin(57600);
  FastLED.addLeds<APA102, SCK, MOSI, BGR, DATA_RATE_MHZ(25)>(leds, N_PIXEL);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 20000);
  //FastLED.addLeds<APA102>(leds, N_PIXEL);//.setCorrection(TypicalSMD5050);
  FastLED.setBrightness(brightness);
  for(int ii = 0; ii < 1; ii++){
    fill(CRGB::Red);
    FastLED.show();
    FastLED.show();
    delay(100);
    fill(CRGB::Black);
    FastLED.show();
    FastLED.show();
    delay(100);
  }
  FastLED.show();
  delay(100);
  Serial.println(N_PIXEL);
  Serial.println("Look for RTC");
  fill(CRGB::Black);
  FastLED.show();
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
  // rainbow test
  for(int ii=0; ii<N_PIXEL; ii++){
    leds[ii] = Wheel(ii % 256);
  }
  FastLED.show();
  while(1);
  */  
  now = rtc.now();
  Serial.println("RTC Found");
  Serial.print("now.unixtime(): ");
  Serial.println(now.unixtime());
  stopwatch_start_time = now.unixtime();
  //stopwatch_start_time = read_stopwatch_start_time();
  if(racing){
    mode = RACE_MODE;
  }
#ifdef NOTDEF  // ## test small font
  Serial.println("test small font");
  // test 4x8 font
  for(int i=0; i<11; i++){
    digit_4x8(i * 5, 0, i % 9, CRGB::White);
  }
  digits_4x8(0, 8, 1234567890L, 11, CRGB::Green);
  FastLED.show();
  uint32_t count = 0;
  while(1){
    digits_4x8(0, 8, count++, 11, CRGB::Green);
    FastLED.show();
    Serial.println("hello");
  }
#endif
  /*
    n_wave = 3;     //default
    wave_sep = 120; // default
    rtc_raw_write(DS3231_ALARM2_OFFSET + 1, 1, false, ((uint8_t*)&n_wave)); // n waves?
    rtc_raw_write(DS3231_ALARM2_OFFSET + 2, 2, false, ((uint8_t*)&wave_sep)); // wave separation?
  */
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

void digit_4x8(byte x, byte y, byte d, const struct CRGB & color){
  byte row, col;
  
  if(d < 10){
    for(col = 0; col < 4; col++){
      for(row = 0; row < 8; row++){
	if((digits4x8[d * 8 + row] >> col) & 1){
	  setPixel(row + y, col + x, color);
	}
	else{
	  setPixel(row + y, col + x, 0);
	}
      }
    }
  }
}

void digits_4x8(byte x, byte y, uint32_t v, byte n_digit, const struct CRGB & color){
  byte digit;
  
  for(byte i = 0; i < n_digit; i++){
    digit = v / int(pow(10, i)) % 10;
    digit_4x8(x + (5 * (n_digit - 1)) - i * 5, y, digit, color);
  }

}

void bigDigit(byte start, byte d, const struct CRGB & color){
  byte row, col;
  if(d < 10){
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
}
void bigDigits(byte start, uint32_t v, const byte n_digit, const struct CRGB & color){
  byte digit;
  
  for(byte i = 0; i < n_digit; i++){
    digit = v / int(pow(10, i)) % 10;
    bigDigit(start + (9 * (n_digit - 1)) - i * 9, digit, color);
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

  if(hh / 10 > 0){
    bigDigit( 0 - 1 + 0, hh / 10, color);
  }
  bigDigit( 9 - 1 + 0, hh % 10, color);
  bigDigit(18 + 1, mm / 10, color);
  bigDigit(27 + 1, mm % 10, color);
  bigDigit(37 + 2, ss / 10, color);
  bigDigit(46 + 2, ss % 10, color);
  if(colen){
    draw_colens(color);
  }
}

int count = 0;
char *msg = "2x6 ULTIM8x8 array!!!   ";


void display_count(unsigned int v, const struct CRGB & color){
  bigDigit( 0, (int)(v / 1e6) % 10, color);
  bigDigit( 8, (int)(v / 1e5) % 10, color);
  bigDigit(16, (int)(v / 1e4) % 10, color);
  bigDigit(24, (int)(v / 1e3) % 10, color);
  bigDigit(32, (int)(v / 1e2) % 10, color);
  bigDigit(40, (int)(v / 1e1) % 10, color);
  bigDigit(48, (int)(v / 1e0) % 10, color);
  FastLED.show();
}
void loop(){
  count++;
  //display_count(count, CRGB::Red);return;
  //displayTime(0, 0, count%10, CRGB::Red, true);  FastLED.show();  return;
  //bigDigits(0, 9999, 4, CRGB::Red);  FastLED.show();  return;
  now = rtc.now();
  updateDisplay();
  //fill(CRGB::White);
  //interact();
  //Serial.println(count);
}

void updateDisplay(){
  uint32_t empty[16];
  uint16_t ii, row, col;
  byte hh, mm, ss;
  bool pending_start = false;
  TimeSpan race_time;
  long long race_seconds;
  uint8_t race_hh, race_mm, race_ss;
  
  fill(CRGB::Black);

  if(now.unixtime() > stopwatch_start_time){
    race_time = TimeSpan(now.unixtime() - stopwatch_start_time);
  }
  else if(mode == RACE_MODE){
    race_time = TimeSpan(stopwatch_start_time - now.unixtime());
    pending_start = true;
  }

  // wall clock time
  hh = now.hour();
  if(USE_12_HOUR_FORMAT){ // use 12 hour clock
    hh = hh % 12;
    if(hh == 0){
      hh = 12;
    }
  }
  mm = now.minute();
  ss = now.second();
  // displayTime(hh % 100, mm, ss, CRGB::White, true); FastLED.show(); return;

  if(mode == STANDBY_MODE){
    // race time
    race_hh = countdown_duration.hours();
    race_mm = countdown_duration.minutes();
    race_ss = countdown_duration.seconds();
    displayTime(race_hh % 100, race_mm, race_ss, CRGB::Yellow, true);
    for(int row=0; row < 16; row++){
      for(int col=0; col < 8; col++){
	setPixel(row, col, CRGB::Black);
      }
    }
  }
  else if(mode == RACE_MODE){
    if(racing){
      // race time
      race_hh = race_time.hours(); //+ race_time.days() * 24;
      race_mm = race_time.minutes();
      race_ss = race_time.seconds();
      race_seconds = race_hh * HOUR + race_mm * MINUTE + race_ss * SECOND;

      displayTime(race_hh % 100, race_mm, race_ss, CRGB::White, race_ss % 2);
      if(pending_start){
	if(0 <= race_seconds && race_seconds < 10){
	  //if(race_hh == 0 && race_mm == 0 && race_ss < 10){
	  if(race_ss > 0){
	    fill(CRGB::Red);
	  }
	  else{
	    fill(CRGB::Green);
	  }
	  for(int col=0; col<10; col++){
	    for(int row=0; row<16; row++){
	      setPixel(row, col+23, CRGB::Black);
	    }
	  }
	  bigDigit(24, race_ss, CRGB::White);
	}
      }
      else{
	for(uint8_t i = 0; i < n_wave; i++){
	  // see if we are about to start a new wave
	  int wave_seconds = race_seconds - i * wave_sep;
	  if(-10 <  wave_seconds && wave_seconds < 0){
	    if(race_ss > 0){
	      fill(CRGB::Red);
	    }
	    else{
	      fill(CRGB::Green);
	    }
	    for(int col=0; col<10; col++){
	      for(int row=0; row<16; row++){
		setPixel(row, col+23, CRGB::Black);
	      }
	    }
	    bigDigit(24, abs(wave_seconds), CRGB::White);
	  }


	  // see if a new wave just started
	  if(0 <= race_seconds - i * wave_sep && race_seconds - i * wave_sep < 10){
	    fill(CRGB::Black);
	    for(int col=0; col<20; col++){
	      for(int row=0; row<16; row++){
		setPixel(row, col+(race_ss%2) * 36, CRGB::Green);
	      }
	    }
	    bigDigit(24, race_seconds - i * wave_sep, CRGB::White);
	  }
	}
      }
    }
    else{ // put dashes
      draw_dashes(CRGB::White);
      draw_colens(CRGB::White);
    }
  }
  else if (mode == CLOCK_MODE){
    displayTime(hh % 100, mm, ss, CRGB::White, true);
  }
  else if (mode == WAVE_MODE){
    bigDigits(0, n_wave, 2, CRGB::Purple);
    if(n_wave > 1){
      bigDigits(56 - 4 * 9, wave_sep % 10000, 4, CRGB::SeaGreen);
    }
  }
  //displayTime(0, 0, count%10, CRGB::Green, true);  FastLED.show();  return;
  FastLED.show();
  Serial.println(ss);
  return;
}

void interact(){
  uint8_t command;
  bool update = false;
  
  while(Serial.available()){
    update = true;
    command = Serial.read();
    Serial.print((char)command);
    if(command > 0 && command <= KANDY_MAX_COMMAND){
      do_command(command);
    }
  }
  while(Serial1.available()){
    update = true;
    command = Serial1.read();
    Serial.write((char)command);
    if(command > 0 && command <= KANDY_MAX_COMMAND){
      do_command(command);
    }
  }
  if(update){
    updateDisplay();
  }
}
void do_command(uint8_t command){
  uint8_t hh = now.hour();
  uint8_t mm = now.minute();
  uint8_t ss = now.second();

  switch(command){
  case KANDY_START: // start / resume
    if(mode == STANDBY_MODE){
      racing = true;
      stopwatch_start_time = (now + countdown_duration).unixtime();
      write_stopwatch_start_time(stopwatch_start_time);
      increment_mode();
    }
    break;
  case KANDY_STOP: // stop / pause
    break;
  case KANDY_INC_HOUR:
    if(mode == CLOCK_MODE){
      now = rtc.now() + HOUR;
      racing = false;
      rtc.adjust(now);
    }
    break;
  case KANDY_DEC_HOUR:
    if(mode == CLOCK_MODE){
      now = rtc.now() - HOUR;
      racing = false;
      rtc.adjust(now);
    }
    break;
  case KANDY_INC_MIN:
    if(mode == CLOCK_MODE){
      now = rtc.now() + MINUTE;
      racing = false;
      rtc.adjust(now);
    }
    break;
  case KANDY_DEC_MIN:
    if(mode == CLOCK_MODE){
      now = rtc.now() - MINUTE;
      racing = false;
      rtc.adjust(now);
    }
    break;
  case KANDY_INC_SEC:
    if(mode == CLOCK_MODE){
      now = rtc.now() + SECOND;
      racing = false;
      rtc.adjust(now);
    }
    break;
  case KANDY_DEC_SEC:
    if(mode == CLOCK_MODE){
      now = rtc.now() - SECOND;
      racing = false;
      rtc.adjust(now);
    }
    break;
  case KANDY_ZERO_SEC:
    if(mode == CLOCK_MODE){
      now = rtc.now() - ss;
      racing = false;
      rtc.adjust(now);
    }
    break;
  case KANDY_INC_CD_HOUR:
    if(mode == STANDBY_MODE){
      countdown_duration = countdown_duration + HOUR;
    }
    break;
  case KANDY_DEC_CD_HOUR:
    if(mode == STANDBY_MODE){
      if(countdown_duration.totalseconds() > 3600){
	countdown_duration = countdown_duration - HOUR;
      }
    }
    break;
  case KANDY_INC_CD_MIN:
    if(mode == STANDBY_MODE){
      countdown_duration = countdown_duration + MINUTE;
    }
    break;
  case KANDY_DEC_CD_MIN:
    if(mode == STANDBY_MODE){
      if(countdown_duration.totalseconds() > 60){
	countdown_duration = countdown_duration - MINUTE;
      }
    }
    break;
  case KANDY_INC_CD_SEC:
    if(mode == STANDBY_MODE){
      countdown_duration = countdown_duration + SECOND;
    }
    break;
  case KANDY_DEC_CD_SEC:
    if(mode == STANDBY_MODE){
      if(countdown_duration.totalseconds() > 0){
	countdown_duration = countdown_duration - SECOND;
      }
    }
    break;
  case KANDY_INC_BRIGHTNESS:
    increment_brightness();
    break;
  case KANDY_DEC_BRIGHTNESS:
    decrement_brightness();
    break;
  case KANDY_INC_MODE:
    increment_mode();
    break;
  case KANDY_DEC_MODE:
    decrement_mode();
    break;
  case KANDY_INC_RACE_HOUR:
    if(mode == RACE_MODE){
      if(stopwatch_start_time > 3600){
	stopwatch_start_time -= 3600;
      }
      else{
	fill(CRGB::Red);
	delay(500);
      }
    }
    break;
  case KANDY_DEC_RACE_HOUR:
    if(mode == RACE_MODE){
      stopwatch_start_time += 3600;
    }
    break;
  case KANDY_INC_RACE_MIN:
    if(mode == RACE_MODE){
      if(stopwatch_start_time > 60){
	stopwatch_start_time -= 60;
      }
      else{
	fill(CRGB::Red);
	delay(500);
      }
    }
    break;
  case KANDY_DEC_RACE_MIN:
    if(mode == RACE_MODE){
      stopwatch_start_time += 60;
    }
    break;
  case KANDY_INC_RACE_SEC:
    if(mode == RACE_MODE){
      if(stopwatch_start_time > 1){
	stopwatch_start_time -= 1;
      }
      else{
	fill(CRGB::Red);
	delay(500);
      }
    }
    break;
  case KANDY_DEC_RACE_SEC:
    if(mode == RACE_MODE){
      stopwatch_start_time += 1;
    }
    break;
  case KANDY_SET_TO_MIDNIGHT:
    if(mode == CLOCK_MODE){
      now = rtc.now() - (hh * HOUR + mm * MINUTE + ss * SECOND);
      racing = false;
      rtc.adjust(now);
    }
    break;
  case KANDY_INC_N_WAVE:
    if(mode == WAVE_MODE){
      if(n_wave < 99){
	n_wave++;
      }
    }
    break;
  case KANDY_DEC_N_WAVE:
    if(mode == WAVE_MODE){
      if(n_wave > 1){
	n_wave--;
      }
    }
    break;
  case KANDY_INC_WAVE_SEP:
    if(mode == WAVE_MODE){
      if(wave_sep < MAX_WAVE_SEP){
	wave_sep+=10;
      }
    }
    break;
  case KANDY_DEC_WAVE_SEP:
    if(mode == WAVE_MODE){
      if(wave_sep > MIN_WAVE_SEP){
	wave_sep-=10;
      }
    }
    break;
  case KANDY_RACE_STOP:
    if(mode == RACE_MODE){
      racing = false;
      write_stopwatch_start_time(stopwatch_start_time); // save race state (fact that race is stopped)
    }
  default:
    break;
  }
}
void increment_brightness(){
  if(brightness < MAX_BRIGHTNESS){
    brightness++;
    FastLED.setBrightness(brightness);
  }
}
void decrement_brightness(){
  if(brightness > 1){
    brightness--;
    FastLED.setBrightness(brightness);
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
  uint16_t countdown = countdown_duration.totalseconds();
  rtc_raw_write(DS3231_ALARM1_OFFSET, 4, false, time_bytes_p);
  rtc_raw_write(DS3231_ALARM2_OFFSET, 1, false, ((uint8_t*)&racing)); // race on?
  rtc_raw_write(DS3231_ALARM2_OFFSET + 1, 1, false, ((uint8_t*)&n_wave)); // n waves?
  rtc_raw_write(DS3231_ALARM2_OFFSET + 2, 2, false, ((uint8_t*)&wave_sep)); // wave separation?
  //rtc_raw_write(DS3231_ALARM2_OFFSET + 4, 2, false, ((uint8_t*)&countdown));
}

time_t read_stopwatch_start_time(){
  uint8_t *time_bytes_p;
  time_t out;
  uint16_t countdown;
  
  /*
    set to:
        0 in clock mode
        start time in race mode
  */
  
  time_bytes_p = (uint8_t*)(&out);  
  rtc_raw_read(DS3231_ALARM1_OFFSET, 4, false, time_bytes_p);
  rtc_raw_read(DS3231_ALARM2_OFFSET, 1, false, ((uint8_t*)&racing)); // race on?
  rtc_raw_read(DS3231_ALARM2_OFFSET + 1, 1, false, ((uint8_t*)&n_wave)); // n waves?
  rtc_raw_read(DS3231_ALARM2_OFFSET + 2, 2, false, ((uint8_t*)&wave_sep)); // n separation?
  rtc_raw_read(DS3231_ALARM2_OFFSET + 4, 2, false, ((uint8_t*)&countdown)); // countdown?
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

