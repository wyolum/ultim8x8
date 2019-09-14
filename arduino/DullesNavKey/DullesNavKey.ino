#include <Time.h>
#include <Wire.h>
#include <WiFiManager.h>
#include <FastLED.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <EEPROMAnything.h>
#include <NTPClient.h>
#include <WebSocketsServer.h>
#include <I2CNavKey.h>
#include <i2cEncoderLibV2.h>
#include "english_v3.h"

//#define ULTIM8x16 // DullesKlok
#include <ESP8266HTTPClient.h>

#include "textures.h"
#include "logic.h"
#include "get_time.h"
#include "config.h"
#include "MatrixMap.h"
#include "digits.h"
#include "fonts.h"
#include "moon_phases.h"

const int IntPin = 16; /* Definition of the interrupt pin*/
//Class initialization with the I2C addresses

// header ################################################################################
void set_timezone_offset(int32_t offset);
uint32_t Now();
void saveSettings();
void display_time(uint32_t last_time, uint32_t current_time);
void apply_mask(bool* mask);
void apply_mask(bool* mask, CRGB color);
void fillMask(bool* mask, bool b);
void fillMask(bool* mask, bool b, int start, int stop);
bool ip_from_str(char* str, byte* ip);

void word_drop_in(uint16_t time_inc, bool* mask);
void word_drop_out(uint16_t time_inc);
void word_drop(uint16_t last_time_inc, uint16_t time_inc);
void fade_transition(uint16_t time_inc, uint16_t last_time_inc);
void plain_jane(uint16_t last_time_inc, uint16_t time_inc);
void TheMatrix(uint16_t last_time_inc, uint16_t time_inc);
void wipe_around_transition(uint16_t last_time_inc, uint16_t time_inc);
void setPixelMask(bool* mask, uint8_t row, uint8_t col, bool b);
void get_time_display(bool* mask, int i);
void getword(int i, uint8_t* out);
void setWordMask(bool *mask, uint8_t* word, bool b);
void prev_display();
void next_display();
void UP_Button_Pressed(i2cNavKey* p);
void DOWN_Button_Pressed(i2cNavKey* p);
void LEFT_Button_Pressed(i2cNavKey* p);
void RIGHT_Button_Pressed(i2cNavKey* p);
void CENTRAL_Button_Pressed(i2cNavKey* p);
void CENTRAL_Button_Double(i2cNavKey* p);
void Encoder_Rotate(i2cNavKey* p);
void fill_color();
void fill_blue();
void fill_red();
void fill_green();
void Wheel(uint8_t WheelPos, uint8_t *red, uint8_t *green, uint8_t *blue);

struct config_t{
  int timezone;
  uint8_t brightness;
  uint8_t display_idx;
  bool factory_reset;
  bool use_wifi;
  bool use_ip_timezone;
  byte mqtt_ip[4];
  bool flip_display;
  uint32_t last_tz_lookup; // look up tz info every Sunday at 3:00 AM
  uint8_t solid_color_rgb[3];
  uint8_t second_color_rgb[3];
  uint8_t third_color_rgb[3];
  bool use_ntp_time;
  bool wifi_reset;
  uint8_t faceplate_idx;
} config;


// end header ################################################################################

// navkey callbacks
//******************************************************************************
i2cNavKey navkey(0b0010000); /* Default address when no jumper are soldered */
void UP_Button_Pressed(i2cNavKey* p) {
  Serial.println("Button UP Pressed!");
  prev_display();
}

void DOWN_Button_Pressed(i2cNavKey* p) {
  Serial.println("Button DOWN Pressed!");
  next_display();
}

void LEFT_Button_Pressed(i2cNavKey* p) {
  Serial.println("Button LEFT Pressed!");
}

void RIGHT_Button_Pressed(i2cNavKey* p) {
  Serial.println("Button RIGHT Pressed!");
}

void CENTRAL_Button_Pressed(i2cNavKey* p) {
  Serial.println("Button Central Pressed!");
}

void CENTRAL_Button_Double(i2cNavKey* p) {
  Serial.println("Button Central Double push!");
}

void Encoder_Rotate(i2cNavKey* p) {
  byte wheel_val = (byte)p->readCounterInt();
  Serial.println(wheel_val);
  Wheel(wheel_val,
	&config.solid_color_rgb[0],
	&config.solid_color_rgb[1],
	&config.solid_color_rgb[2]);
  saveSettings();
}
// end navkey callbacks
//********************************************************************************

WiFiManager wifiManager;
WiFiUDP ntpUDP;
/*
Sunday
Monday
Tuesday
Wednesday
Thursday
Friday
Saturday
 */

const uint8_t N_BRIGHTNESS = 18;
const uint8_t BRIGHTNESSES[N_BRIGHTNESS] = {1, 2,  3,  4,  5,  6,  7,  8,
					    10, 12, 14, 16, 19, 23, 27, 32, 38, 45};

bool mask[NUM_LEDS];
bool wipe[NUM_LEDS];
CRGB leds[NUM_LEDS];
CRGB buffer[2 * NUM_LEDS];

// language constants
uint8_t n_byte_per_display = 4;         // number of bytes used for each 5 minunte time incriment
uint8_t display_idx;
uint8_t last_min_hack_inc = 0;
uint16_t last_time_inc = 0;

NTPClient timeClient(ntpUDP, "us.pool.ntp.org", 0, 60000); // do not use directly (only through ntp_clock)
NTPClock ntp_clock; // uses timeClient


void getword(int i, uint8_t* out){
  out[0] = pgm_read_byte(WORDS + 3 * i + 1);
  out[1] = pgm_read_byte(WORDS + 3 * i + 2);
  out[2] = pgm_read_byte(WORDS + 3 * i + 3);
}
void setWordMask(bool *mask, uint8_t* word, bool b){ 
 // word = [row, col, len]
  for(int i=0; i < word[2]; i++){
    setPixelMask(mask, word[1], word[0] + i, b);
  }
}

typedef void (*Init)();
typedef void (*Background)();
typedef void (*Transition)(uint16_t last_time_inc, uint16_t time_inc);
typedef struct{
  Init       init;
  Background background;
  Transition transition;
  String     name;
  int        id;
} Display;

typedef Display Displays[];

void blend_to_rainbow();

Display WordDropDisplay = {blend_to_rainbow, rainbow, word_drop, String("Word Drop"), 0};
Display WipeAroundDisplay = {blend_to_rainbow, rainbow, wipe_around_transition, String("Wipe Around"), 1};
Display TheMatrixDisplay = {fill_blue, fill_blue, TheMatrix, String("The Matrix"), 2};
Display PlaneJaneDisplay = {fill_color, fill_color, plain_jane, String("Plane Jane"), 3};

const int N_DISPLAY = 4;
Display *Display_ps[N_DISPLAY] = {&WordDropDisplay,
				  &WipeAroundDisplay,
				  &TheMatrixDisplay,
				  &PlaneJaneDisplay};
//Display* CurrentDisplay_p = &WipeAroundDisplay;
Display* CurrentDisplay_p = &TheMatrixDisplay;
//Display* CurrentDisplay_p = &WordDropDisplay;

void get_time_display(bool* mask, int time_inc){
  uint8_t bits;     // holds the on off state for 8 words at a time
  uint8_t word[3];  // start columm, start row, length of the current word

  for(uint8_t j = 0; j < n_byte_per_display; j++){ // j is a byte index 
    // read the state for the next set of 8 words
    bits = pgm_read_byte(DISPLAYS + 1 + (time_inc * n_byte_per_display) + j);
    for(uint8_t k = 0; k < 8; k++){                     // k is a bit index
      if((bits >> k) & 1){                              // check to see if word is on or off
	getword(j * 8 + k, word);                       // if on, read location and length
	for(int m=word[0]; m < word[0] + word[2]; m++){ // and display it
	  setPixelMask(mask, word[1], m, true);
	}
      }
    }
  }
}

void word_drop_in(uint16_t time_inc, bool* mask){
  uint8_t bits;     // holds the on off state for 8 words at a time
  uint8_t word[3];  // start columm, start row, length of the current word
  bool tmp_mask[NUM_LEDS];
  uint8_t tmp_word[3];
  
  fillMask(mask, false);
  fillMask(wipe, false);
  fillMask(tmp_mask, false);
  
  for(uint8_t j = 0; j < n_byte_per_display; j++){ // j is a byte index 
    // read the state for the next set of 8 words
    bits = pgm_read_byte(DISPLAYS + 1 + (time_inc * n_byte_per_display) + j);
    for(uint8_t k = 0; k < 8; k++){                     // k is a bit index
      if((bits >> k) & 1){                              // check to see if word is on or off
	getword(j * 8 + k, word);                       // if on, read location and length
	tmp_word[0] = word[0];
	tmp_word[1] = word[1];
	tmp_word[2] = word[2];
	for(int rr = 0; rr <= word[1]; rr++){
	  tmp_word[1] = rr;
	  setWordMask(wipe, tmp_word, true);
	  logical_or(NUM_LEDS, mask, wipe, tmp_mask);
	  rainbow();
	  apply_mask(tmp_mask);
	  FastLED.show();
	  delay(25);
	}
	setWordMask(mask, word, true);
	for(int rr = 0; rr < word[1]; rr++){
	  tmp_word[1] = rr;
	  setWordMask(wipe, tmp_word, false);
	  logical_or(NUM_LEDS, mask, wipe, tmp_mask);
	  
	  rainbow();
	  apply_mask(tmp_mask);
	  FastLED.show();
	  delay(25);
	}
      }
    }
  }
}
void word_drop_out(uint16_t time_inc){
  uint8_t bits;     // holds the on off state for 8 words at a time
  uint8_t word[3];  // start columm, start row, length of the current word
  bool tmp_mask[NUM_LEDS];
  uint8_t tmp_word[3];
  
  //fillMask(mask, false);
  //fillMask(wipe, false);
  //fillMask(tmp_mask, false);
  logical_copy(NUM_LEDS, mask, wipe);
  logical_copy(NUM_LEDS, mask, tmp_mask);
  
  for(uint8_t j = 0; j < n_byte_per_display; j++){ // j is a byte index 
    // read the state for the next set of 8 words
    bits = pgm_read_byte(DISPLAYS + 1 + (time_inc * n_byte_per_display) + j);
    for(uint8_t k = 0; k < 8; k++){                     // k is a bit index
      if((bits >> k) & 1){                              // check to see if word is on or off
	getword(j * 8 + k, word);                       // if on, read location and length
	tmp_word[0] = word[0];
	tmp_word[1] = word[1];
	tmp_word[2] = word[2];
	for(int rr = word[1]; rr <= 8; rr++){
	  tmp_word[1] = rr;
	  setWordMask(wipe, tmp_word, true);
	  logical_or(NUM_LEDS, mask, wipe, tmp_mask);
	  rainbow();
	  apply_mask(tmp_mask);
	  FastLED.show();
	  delay(25);
	}
	setWordMask(mask, word, false);
	for(int rr = word[1]; rr <= 8; rr++){
	  tmp_word[1] = rr;
	  setWordMask(wipe, tmp_word, false);
	  logical_or(NUM_LEDS, mask, wipe, tmp_mask);
	  
	  rainbow();
	  apply_mask(tmp_mask);
	  FastLED.show();
	  delay(25);
	}
      }
    }
  }
}

void word_drop(uint16_t last_time_inc, uint16_t time_inc){
  rainbow();

  // swipe rainbow from the left
  //wipe_around(ON);
  //delay(1000);
  if(last_time_inc != 289){
    word_drop_out(last_time_inc);
  }
  
  // clear the new display
  fillMask(mask, false);
  
  // read display for next time incement
  get_time_display(mask, time_inc);
  
  // clear rainbow to reveal the time
  //wipe_off_left();
  //wipe_around(OFF);
  word_drop_in(time_inc, mask);
}

void fade_transition(uint16_t time_inc, uint16_t last_time_inc){
  int i, j;
  
  fillMask(mask, false);
  get_time_display(mask, last_time_inc);
  rainbow();
  apply_mask(mask); // fill leds with next time inc and copy to second half of buffer
  for(j = 0; j < NUM_LEDS; j++){
    buffer[j] = leds[j]; // buffer has new time
  }

  fillMask(mask, false);
  get_time_display(mask, time_inc); // leds have old time
  rainbow();
  apply_mask(mask);

  int rate = 1;
  for(j = 0; j < 255; j++){
    for(i=0; i < NUM_LEDS; i++){
      leds[i].red   = blend8(leds[i].red,   buffer[i].red, rate);
      leds[i].green = blend8(leds[i].green, buffer[i].green, rate);
      leds[i].blue  = blend8(leds[i].blue,  buffer[i].blue, rate);
    }
    FastLED.show();
    delay(1);
  }
}

void plain_jane(uint16_t last_time_inc, uint16_t time_inc){
  fillMask(mask, false);
  get_time_display(mask, time_inc); // leds have old time
  fill_color();
  apply_mask(mask);
}
void TheMatrix(uint16_t last_time_inc, uint16_t time_inc){
  int n_drop = 0;
  int n_need = 8;
  
  const struct CRGB color = CRGB::Green;
  uint8_t cols[NUM_LEDS];
  uint8_t rows[NUM_LEDS];
  uint8_t pause[NUM_LEDS];
  bool have[NUM_LEDS];
  int col;
  int i, j;

  // rainbow();
  fillMask(mask, false);
  fillMask(wipe, false);
  fillMask(have, false);
  get_time_display(mask, last_time_inc);
  get_time_display(wipe, time_inc);
  apply_mask(mask);
  FastLED.show();
  for(i=0; i < MatrixWidth; i++){
    for(j=0; j < MatrixHeight; j++){
      if(leds[XY(i, j)].red > 0 ||
	 leds[XY(i, j)].green > 0 ||
	 leds[XY(i, j)].blue > 0){
	rows[n_drop] = j;
	cols[n_drop] = i;
	n_drop++;
      }
      if(wipe[XY(i, j)]){
	n_need++;
      }
    }
  }
  
  delay(10);
  for(j = 0; j < 255 * 3; j++){
    for(i=0; i < NUM_LEDS; i++){
      leds[i].red   = blend8(leds[i].red, 0, 1);
      leds[i].green = blend8(leds[i].green, 255, 1);
      leds[i].blue  = blend8(leds[i].blue, 0, 1);
    }
    apply_mask(mask);
    FastLED.show();
    delay(5);
  }

  for(i = n_drop; i < n_need; i++){/// add enough drops to complete
    cols[i] = random(0, MatrixWidth);
    rows[i] = -random(0, MatrixHeight);
    n_drop++;
  }

  int end = millis() + 5000; // go for 5 seconds
  // while new display is not filled out
  while(!logical_equal(NUM_LEDS, wipe, have)){
      //  while(millis() < end){
    fadeToBlackBy(leds, NUM_LEDS, 75);
    for(i = 0; i < n_drop; i++){
      if(millis() > end && wipe[XY(cols[i], rows[i])]){
	if(random(0, 3) == 0){
	  have[XY(cols[i], rows[i])] = true;
	}
      }
      
      if(random(0, 16) == 0){ // pause at random times
	pause[i] = random(6, 9); // for random duration
      }
      if(pause[i] == 0){
	rows[i]++;
      }
      else{
	pause[i]--; 
      }
      if(rows[i] > MatrixHeight - 1){
	if(n_drop > n_need){
	  for(j = i; j < n_drop; j++){ // slide drops down by one
	    rows[j] = rows[j + 1];
	    cols[j] = cols[j + 1];
	  }
	  n_drop--;
	  Serial.print("n_drop:");
	  Serial.println(n_drop);
	}
	else{
	  rows[i] = -random(0, MatrixHeight);
	  cols[i] = random(0, MatrixWidth);
	}
      }
      if(0 <= rows[i] && rows[i] <  MatrixHeight){
	leds[XY(cols[i], rows[i])] = color;
      }
    }

    for(int ii = 0; ii < NUM_LEDS; ii++){
      if(have[ii]){
	leds[ii] = CRGB::Blue;
      }
    }
    FastLED.show();
    delay(75);
  }
  for(int ii=0; ii< MatrixHeight * 10; ii++){
      //  while(millis() < end){
    fadeToBlackBy(leds, NUM_LEDS, 75);
    for(i = 0; i < n_drop; i++){
      rows[i]++;
      if(0 <= rows[i] && rows[i] <  MatrixHeight){
	leds[XY(cols[i], rows[i])] = color;
      }
    }
    for(int ii = 0; ii < NUM_LEDS; ii++){
      if(have[ii]){
	leds[ii] = CRGB::Blue;
      }
    }
    FastLED.show();
    delay(75);
  }

  // fade to green
  delay(1000);
  
  
  
}

void wipe_around(bool val){
  float dtheta = 31.4 / 180;
  float theta = -3.14 - dtheta;
  int row, col;
  bool tmp[NUM_LEDS];

  int cx = random(0, MatrixWidth-1);
  int cy = random(0, MatrixHeight-1);
  cx = 8;
  cy = 4;
  
  fillMask(wipe, !val);
  while (theta < 3.14 + dtheta){
    for(row=0; row < MatrixHeight; row++){
      for(col=0; col < MatrixWidth; col++){
	if(atan2(row - cy, col - cx) < theta){
	  setPixelMask(wipe, row, col, val);
	}
      }
    }
    logical_or(NUM_LEDS, wipe, mask, tmp);    
    rainbow();
    apply_mask(tmp);
    FastLED.show();
    theta += dtheta;
  }
}

void wipe_around_transition(uint16_t last_time_inc, uint16_t time_inc){
  rainbow();
  fillMask(mask, false);
  get_time_display(mask, last_time_inc);
  apply_mask(mask);
  wipe_around(ON);
  delay(10);

  fillMask(mask, false);
  get_time_display(mask, time_inc);
  apply_mask(mask);
  wipe_around(OFF);
  delay(10);
}
void wipe_down(bool val){
  int col, row;
  bool tmp[NUM_LEDS];
  
  fillMask(wipe, !val);
  for(row=0; row < MatrixHeight; row++){
    for(col = MatrixWidth - 1; col >= 0; col--){
      setPixelMask(wipe, row, col, val);
    }
    logical_or(NUM_LEDS, wipe, mask, tmp);
    rainbow();
    apply_mask(tmp);
    FastLED.show();
    delay(20);
  }
}

void wipe_up(bool val){
  int col, row;
  bool tmp[NUM_LEDS];
  
  fillMask(wipe, !val);
  for(row = MatrixHeight - 1; row >= 0; row--){
    for(col = MatrixWidth - 1; col >= 0; col--){
      setPixelMask(wipe, row, col, val);
    }
    logical_or(NUM_LEDS, wipe, mask, tmp);
    rainbow();
    apply_mask(tmp);
    FastLED.show();
    delay(20);
  }
}

void wipe_left(bool val){
  int col, row;
  bool tmp[NUM_LEDS];
  
  fillMask(wipe, !val);
  for(col = MatrixWidth - 1; col >= 0; col--){
    for(row=0; row < MatrixHeight; row++){
      setPixelMask(wipe, row, col, val);
    }
    logical_or(NUM_LEDS, wipe, mask, tmp);
    rainbow();
    apply_mask(tmp);
    FastLED.show();
    delay(10);
  }
}

void next_display(){
  Serial.println("Next Display");
  String name = CurrentDisplay_p->name;
  Serial.print("from:");
  Serial.print(name);
  int next_display_i = (CurrentDisplay_p->id + 1) % N_DISPLAY;
  CurrentDisplay_p = Display_ps[next_display_i];
  CurrentDisplay_p->init();
  Serial.print(" to:");
  Serial.println(CurrentDisplay_p->name);
}
void prev_display(){
  Serial.println("Previous Display");
  String name = CurrentDisplay_p->name;
  Serial.print("from:");
  Serial.print(name);
  int prev_display_i = (CurrentDisplay_p->id + N_DISPLAY - 1) % N_DISPLAY;
  CurrentDisplay_p = Display_ps[prev_display_i];
  CurrentDisplay_p->init();
  Serial.print(" to:");
  Serial.println(CurrentDisplay_p->name);
}


void Wheel(uint8_t WheelPos, uint8_t *red, uint8_t *green, uint8_t *blue) {
  uint8_t r, g, b;
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    r = 255 - WheelPos * 3;
    g = 0;
    b = WheelPos * 3;
  }
  else if(WheelPos < 170) {
    WheelPos -= 85;
    r = 0;
    g = WheelPos * 3;
    b = 255 - WheelPos * 3;
  }
  else{
    WheelPos -= 170;
    r = WheelPos * 3;
    g = 255 - WheelPos * 3;
    b = 0;
  }
  *red = r;
  *green = g;
  *blue = b;
}

void fillMask(bool val, bool *mask){
  for(int i=0;i<NUM_LEDS; i++){
    mask[i] = val;
  }
}

void maskPixel(byte row, byte col, bool val, bool *mask){
  uint16_t pos = XY(row, col);
  if(0 <= pos && pos < NUM_LEDS){
    mask[pos] = val;
  }
}

void print_time(){
  Serial.print("Time: ");
  Serial.print(ntp_clock.year());
  Serial.print("/");
  Serial.print(ntp_clock.month());
  Serial.print("/");
  Serial.print(ntp_clock.day());
  Serial.print(" ");
  if(ntp_clock.hours() < 10)Serial.print('0');
  Serial.print(ntp_clock.hours());
  Serial.print(":");
  if(ntp_clock.minutes() < 10)Serial.print('0');
  Serial.print(ntp_clock.minutes());
  Serial.print(":");
  if(ntp_clock.seconds() < 10)Serial.print('0');
  Serial.println(ntp_clock.seconds());
}
void print_config(){
  Serial.println("config:");
  Serial.print("    timezone:"); Serial.println(config.timezone);
  Serial.print("    brightness:"); Serial.println(config.brightness);
  Serial.print("    display_idx:"); Serial.println(config.display_idx);
  Serial.print("    factory_reset:"); Serial.println(config.factory_reset);
  Serial.print("    use_wifi:"); Serial.println(config.use_wifi);
  Serial.print("    use_ip_timezone:"); Serial.println(config.use_ip_timezone);
  Serial.println();
  Serial.print("    flip_display:"); Serial.println(config.flip_display);
  Serial.print("    last_tz_lookup:"); Serial.println(config.last_tz_lookup);
  Serial.print("    solid_color_rgb:");
  for(int ii = 0; ii < 3; ii++){
    if (ii > 0){
      Serial.print(".");
    }
    Serial.print(config.solid_color_rgb[ii]);
  }
  Serial.print("    second_color_rgb:");
  for(int ii = 0; ii < 3; ii++){
    if (ii > 0){
      Serial.print(".");
    }
    Serial.print(config.second_color_rgb[ii]);
  }
  Serial.print("    third_color_rgb:");
  for(int ii = 0; ii < 3; ii++){
    if (ii > 0){
      Serial.print(".");
    }
    Serial.print(config.third_color_rgb[ii]);
  }
  Serial.println();
  Serial.print("    use_ntp_time:"); Serial.println(config.use_ntp_time);
  Serial.print("    wifi_reset:"); Serial.println(config.wifi_reset);
  Serial.print("    faceplate_idx:"); Serial.println(config.faceplate_idx);
}

//const bool ON = true;
//const bool OFF = !ON;

// How many leds are in the strip?
const uint8_t N_BOARD = 2;

WiFiClient espClient;
void  interact_loop();

#define DATA_PIN     MOSI
#define CLK_PIN      SCK

#define COLOR_ORDER BGR
#define LED_TYPE APA102
#define MILLI_AMPS 2000  // IMPORTANT: set the max milli-Amps of your power supply (4A = 4000mA)

uint32_t last_time;
bool sleeping = false;

//********************************************************************************
// Displays

void my_show(){
  FastLED.show();
  interact_loop();
}

typedef void (*Init)();
typedef void (*DisplayTime)(uint32_t last_tm, uint32_t tm);

String jsonLookup(String s, String name){
  int start = s.indexOf(name) + name.length() + 3;
  int stop = s.indexOf('"', start);
  Serial.println(s.substring(start, stop));
  return s.substring(start, stop);
}

void set_timezone_from_ip(){

  HTTPClient http;
  
  Serial.print("[HTTP] begin...\n");
  // configure traged server and url
  //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
  // http.begin("http://example.com/index.html"); //HTTP

  //http.begin("https://ipapi.co/json");
  String url = String("http://www.wyolum.com/utc_offset/utc_offset.py") +
    String("?refresh=") + String(millis()) +
    String("&localip=") +
    String(WiFi.localIP()[0]) + String('.') + 
    String(WiFi.localIP()[1]) + String('.') + 
    String(WiFi.localIP()[2]) + String('.') + 
    String(WiFi.localIP()[3]) + String('&') +
    String("macaddress=") + WiFi.macAddress() + String('&') + 
    String("dev_type=ClockIOT");
  Serial.println(url);
  http.begin(url);
  
  Serial.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();
  
  // httpCode will be negative on error
  if(httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    
    // file found at server
    //String findme = String("offset_seconds");
    if(httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.print("payload:");
      Serial.println(payload);
      payload.replace(" ", "");
      String offset_str = jsonLookup(payload, String("utc_offset"));
      int hours = offset_str.substring(0, 3).toInt();
      int minutes = offset_str.substring(3, 5).toInt();
      if(hours < 0){
	minutes *= -1;
      }
      int offset = hours * 3600 + minutes * 60;

      if(config.use_ip_timezone){
	Serial.print("timezone_offset String:");
	Serial.println(offset_str);
	Serial.print("timezone_offset:");
	Serial.println(offset);
	set_timezone_offset(offset);
	config.last_tz_lookup = Now();
	saveSettings();
      }
      else{
	Serial.println("Using previously selected timezone");
      }
    }
    else{
      Serial.println("No timezone found");
    }
  }
  else{
    Serial.print("[HTTP]    ... Failed:");
    Serial.println(httpCode);
  }
}

void setPixel(byte row, byte col, const struct CRGB & color){
  int pos = XY(col, row);
  leds[pos] = color;
}
void setPixelMask(bool* mask, uint8_t row, uint8_t col, bool b){
  if(row >= MatrixHeight){
  }
  else if(col >= MatrixWidth){
  }
  else{
    uint16_t pos = XY(col, row);
    if(pos < NUM_LEDS){
      mask[pos] = b;
    }
  }
}

//--------------------------------------------------------------------------------
//uint32_t current_time;
void blend_to_rainbow(){
  int i;
  CHSV newcolor;
  uint32_t current_time = Now();
  //current_time = Now();
  int count = ((current_time % 300) * 255) / 300;
  
  newcolor.val = 255;
  newcolor.sat = 255;
  for(int ii=0; ii<NUM_LEDS; ii++){
    for( int row = 0; row < MatrixHeight; row++) {
      for( int col = 0; col < MatrixWidth; col++) {
	i = XY(col, row);
	if(mask[i]){
	  newcolor.hue =  (count + (MatrixWidth * row + col) * 2) % 256;
	  nblend(leds[XY(col, row)], newcolor, 1);
	}
      }
    }
    my_show();
    delay(1);
  }
}

void blend_to_color(CRGB color){
  for(int kk=0; kk<128; kk++){
    for(int ii=0; ii<NUM_LEDS; ii++){
      if(mask[ii]){
	nblend(leds[ii], color, 1);
      }
    }
    my_show();
    delay(1);
  }
}

void blend_to_red(){
  blend_to_color(CRGB::Red);
}

void blend_to_green(){
  blend_to_color(CRGB::Green);
}

void blend_to_blue(){
  blend_to_color(CRGB::Blue);
}

void fill_color(){
  fill_solid(leds, NUM_LEDS, CRGB(config.solid_color_rgb[0],
  				  config.solid_color_rgb[1],
  				  config.solid_color_rgb[2]));
}
void fill_red(){
  fill_solid(leds, NUM_LEDS, CRGB::Red);
}
void fill_green(){
  fill_solid(leds, NUM_LEDS, CRGB::Green);
}
void fill_blue(){
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
}
void fill_white(){
  fill_solid(leds, NUM_LEDS, CRGB::White);
}
void fill_black(){
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}
void noop(){
}

uint8_t logo_rgb[] = {
  0x11,0x00,0x29,0x00,0x25,0x00,0x23,0x00,0x25,0x00,0x29,0x00,0x31,0x00,0xe0,0x01,
  0x00,0x03,0x80,0x04,0x80,0x04,0x80,0x04,0x80,0x04,0x00,0x03,0x00,0x00,0x00,0x00,
  0x11,0x00,0x09,0x88,0x05,0x48,0x03,0x28,0x05,0x18,0x09,0x28,0x11,0x48,0x00,0x88
};

void set_brightness(uint8_t brightness){
  if(brightness < N_BRIGHTNESS){
    config.brightness = brightness;
    FastLED.setBrightness(BRIGHTNESSES[config.brightness]);
    Serial.print("Adjust brightness to ");
    Serial.println(config.brightness);
    saveSettings();
  }
}

void dimmer(){
  byte b;
  b = config.brightness;
  if(b >= 1){
    b -= 1;
  }
  set_brightness(b);
}
void brighter(){
  byte b;
  b = config.brightness;
  if(b < N_BRIGHTNESS - 1){
    b += 1;
  }
  set_brightness(b);
}

void add_to_timezone(int32_t offset){ 
  config.timezone += offset;
  config.use_ip_timezone = false; // time zone manually changed... ignore internate timezone
  saveSettings();
  if(config.use_wifi){
    ntp_clock.setOffset(config.timezone);
  }
}

void set_timezone_offset(int32_t offset){
  config.timezone = offset % 86400;
  saveSettings();
  if(config.use_wifi){
    ntp_clock.setOffset(config.timezone);
  }
}

void display_bitmap_rgb(uint8_t* bitmap){
  uint8_t n = 16;
  uint8_t h = 8;
  uint8_t w = 16;
  uint8_t xy[2];
  uint8_t r, g, b;
  
  int i, j;
  int led_idx, byte_idx;

  //struct CRGB color;
  int x, y;
  
  for(i=0; i<n; i++){
    r = bitmap[i + 0 * w];
    g = bitmap[i + 1 * w];
    b = bitmap[i + 2 * w];
    //   012345678pabcdef
    // 0 0000000011111111
    // 1 2222222233333333
    // 2 4444444455555555
    // 3 6666666677777777
    // 4 8888888899999999
    // 5 aaaaaaaabbbbbbbb
    // 6 ccccccccdddddddd
    // 7 eeeeeeeeffffffff
    for(j=0; j<8; j++){
      x = (i * 8 + j) % 16;
      y = i / 2;
      led_idx = XY(x, y);
      leds[led_idx].red   = 255 * ((r >> j) & 1);
      leds[led_idx].green = 255 * ((g >> j) & 1);
      leds[led_idx].blue  = 255 * ((b >> j) & 1);
      if(leds[led_idx].red || leds[led_idx].green || leds[led_idx].blue){
	mask[led_idx] = true;
      }
    }
  }
}

void apply_mask(bool* mask){
  uint16_t b, k;
  for(uint16_t i=0; i < NUM_LEDS; i++){
    if(!mask[i]){
      leds[i] = CRGB::Black;
    }
  }
}
void apply_mask(bool* mask, CRGB color){
  uint16_t b, k;
  for(uint16_t i=0; i < NUM_LEDS; i++){
    if(!mask[i]){
      leds[i] = color;
    }
  }
}

void fillMask(bool* mask, bool b){
  fillMask(mask, b, 0, NUM_LEDS);
}

void fillMask(bool* mask, bool b, int start, int stop){
  for(int i = start; i < stop && i < NUM_LEDS; i++){
    mask[i] = b;
  }
}

void loadSettings(){
  EEPROM_readAnything(0, config);
}

void saveSettings(){
  EEPROM_writeAnything(0, config);
  EEPROM.commit();
}

void paintMask(bool* mask, CRGB color){
  uint16_t b, k;
  for(uint16_t i=0; i < NUM_LEDS; i++){
    if(mask[i]){
      leds[i] = color;
    }
  }
}

void bigDigit(int x, int y, byte digit, bool* mask){
  byte row, col;
  if(digit < 10){
    for(col = 0; col < 8; col++){
      for(row = 0; row < 16; row++){
	if((digits8x16[digit * 16 + row] >> col) & 1){
	  maskPixel(col + x, row + y, true, mask);
	}
	else{
	  maskPixel(col + x, row + y, false, mask);
	}
      }
    }
  }
}

void middleDigit(int x, int y, byte digit, bool* mask){
  byte row, col;
  if(digit < 10){
    for(col = 0; col < 7; col++){
      for(row = 0; row < 11; row++){
	int _row = 11 - row - 1;
	int _col = 7 - col - 1;
	if((digits7x11[digit * 11 + _row] >> _col) & 1){
	  maskPixel(col + x, row + y, true, mask);
	}
	else{
	  maskPixel(col + x, row + y, false, mask);
	}
      }
    }
  }
}

void moonPhase(int x, int y, bool* mask){
  uint8_t row, col;
  uint8_t phase_idx = 0;
  uint8_t i;
  const uint8_t *phase;
  double theta = get_moon_phase(ntp_clock.gmt());
  
  int theta_byt = (int)((theta * 256) / (2 * PI));
  // find phase index
  for(i = 1; i < N_MOON_PHASE; i++){
    if(moon_phases_11x11[i * MOON_PHASE_STEP] > theta_byt){
      phase_idx = i - 1;
      break;
    }
  }
  uint8_t bit, byt;
  uint16_t n; // pixel counter
  bool val;
  phase = &moon_phases_11x11[phase_idx * MOON_PHASE_STEP];
  if(phase_idx < N_MOON_PHASE){
    for(col = 0; col < 11; col++){
      for(row = 0; row < 11; row++){
	n = row * 11 + col;
	byt = n / 8 + 1; // plus one for theta_byt
	bit = n % 8;
	if (byt < MOON_PHASE_STEP){
	  val = (phase[byt] >> bit) & 1;
	  maskPixel(col + x, row + y, val, mask);
	}
      }
    }
    maskPixel(x + 10, y + 10, false, mask); // last always off
  }
}

void littleDigit(int x, int y, byte digit, bool* mask){
  byte row, col;
  if(digit < 10){
    for(col = 0; col < 4; col++){
      for(row = 0; row < 7; row++){
	if((digits4x7[digit * 4 + col] >> row) & 1){
	  maskPixel(col + x, row + y, true, mask);
	}
	else{
	  maskPixel(col + x, row + y, false, mask);
	}
      }
    }
  }
}

void littleChar(uint8_t x, uint8_t y, char c, bool *mask){
  uint8_t idx = (uint8_t)c - 32;
  byte row, col;
  const unsigned char* pixels = font_5x6[idx];
  
  if(idx < 96){
    for(col = 0; col < 5; col++){
      for(row = 0; row < 5; row++){
	if((pixels[col] >> (row+2)) & 1){
	  maskPixel(col + x, row + y, true, mask);
	}
	else{
	  maskPixel(col + x, row + y, false, mask);
	}
      }
    }
  }
}

void bigChar(uint8_t x, uint8_t y, char c, bool *mask){
  uint8_t idx = (uint8_t)c - 32;
  byte row, col;
  const unsigned char* pixels = font_7x13[idx];
  
  if(idx < 96){
    for(col = 0; col < 7; col++){
      for(row = 0; row < 13; row++){
	if((pixels[12-row] >> (7-col)) & 1){
	  maskPixel(col + x, row + y, true, mask);
	}
	else{
	}
      }
    }
  }
}

uint8_t hex2dig(char h){
  uint8_t d = 0;
  if('0' <= h && h <= '9'){
    d = (uint8_t)(h - '0');
  }
  else if('a' <= h && h <= 'f'){
    d = (uint8_t)(10 + h - 'a');
  }
  else if('A' <= h && h <= 'F'){
    d = (uint8_t)(10 + h - 'A');
  }
  return d;
}

uint8_t hh2dd(char *hh){
  return hex2dig(hh[0]) * 16 + hex2dig(hh[1]);
}

bool force_timezone_from_ip = false;

void handle_msg(char* topic, byte* payload, unsigned int length) {
  bool handled = false;
  char str_payload[length + 1];
  char *subtopic = topic + 9;

  // copy bytes to normal char array
  for(int i = 0; i < length; i++){
    str_payload[i] = payload[i];
  }
  str_payload[length] = 0;
  
  Serial.print("msg\n  topic:");
  Serial.print(topic);
  Serial.print(" subtopic:");
  Serial.println(subtopic);
  Serial.print("  payload:");
  Serial.println(str_payload);
  Serial.print("  length:");
  Serial.println(length);
  
  if(strcmp(subtopic, "timezone_offset") == 0){
    Serial.println("Change timezone.");
    if(strcmp(str_payload, "IP") == 0){
      config.use_ip_timezone = true;
      force_timezone_from_ip = true;
      saveSettings();
      print_config();
    }
    else{
      String s = String(str_payload);
      set_timezone_offset(s.toInt());
      config.use_ip_timezone = false;
      saveSettings();
      print_config();
    }
  }
  else if(strcmp(subtopic, "add_to_timezone") == 0){
    Serial.println("Add to timezone!");
    add_to_timezone(String(str_payload).toInt());
  }
  else if(strcmp(subtopic, "display_idx") == 0){
    Serial.println("Change display_idx, not supported.");
  }
  else if(strcmp(subtopic, "prev_display") == 0){
    config.display_idx = (config.display_idx + N_DISPLAY - 1) % N_DISPLAY;
    saveSettings();
    Serial.print("Display incremented to: ");
    Serial.print(config.display_idx);
  }
  else if(strcmp(subtopic, "next_display") == 0){
    config.display_idx = (config.display_idx + 1) % N_DISPLAY;
    saveSettings();
    Serial.print("Display incremented to: ");
    Serial.print(config.display_idx);
  }
  else if(strcmp(subtopic, "toggle_sleep") == 0){
    sleeping = !sleeping;
    if(sleeping){
      Serial.println("Going to sleep.");
    }
    else{
      Serial.println("Waking up.");
    }
  }
  else if(strcmp(subtopic, "brighter") == 0){
    Serial.println("Increment brigtness.");
    brighter();
  }
  else if(strcmp(subtopic, "dimmer") == 0){
    Serial.println("Decrement brigtness.");
    dimmer();
    Serial.println(config.brightness);
  }
  else if(strcmp(subtopic, "flip_display") == 0){
    Serial.print("Flip Display not supported");
  }
  else if(strcmp(subtopic, "set_rgb") == 0 && length >= 6){
    // payload: rrggbb lowercase html color code example "ff0000" is RED
    Serial.print("color changed to: ");
    for(int i=0; i<6; i++){
      Serial.print((char)payload[i]);
    }
    config.solid_color_rgb[0] = hh2dd((char*)payload);
    config.solid_color_rgb[1] = hh2dd((char*)payload + 2);
    config.solid_color_rgb[2] = hh2dd((char*)payload + 4);
    saveSettings();
  }
  else if(strcmp(subtopic, "set_colorwheel") == 0 && length >= 1){
    // payload: 0-255 color wheel value
    Serial.print("color changed to wheel:");
    for(int i=0; i<length; i++){
      Serial.print((char)payload[i]);
    }

    uint8_t wheel_val = String(str_payload).toInt();
    Wheel(wheel_val,
	  &config.solid_color_rgb[0],
	  &config.solid_color_rgb[1],
	  &config.solid_color_rgb[2]);
    Wheel(wheel_val + 85,
	  &config.second_color_rgb[0],
	  &config.second_color_rgb[1],
	  &config.second_color_rgb[2]);
    Wheel(wheel_val + 85 * 2,
	  &config.third_color_rgb[0],
	  &config.third_color_rgb[1],
	  &config.third_color_rgb[2]);
    saveSettings();
  }
  else if(strcmp(subtopic, "set_time") == 0){
    uint32_t tm = String(str_payload).toInt();
    config.use_ip_timezone = false;
    config.use_ntp_time = false;
    saveSettings();
  }
  else if(strcmp(subtopic, "use_ntp") == 0){
    if(!config.use_ntp_time){
      Serial.println("Use NTP time service.");
      // turn ON internet time
      config.use_ip_timezone = true;
      config.use_wifi = true;
      config.use_ntp_time = true;

      ntp_clock.setup(&timeClient);
      ntp_clock.setOffset(config.timezone);
      saveSettings();
    }
    else{
      Serial.println("Already using NTP time service.");
    }
  }
  else if(strcmp(subtopic, "notify") == 0){
    // payload: ascii notification
  }
}

bool ip_from_str(char* str, byte* ip){
  byte my_ip[4];
  int end_poss[4];
  int i = 0, j = 0;
  int dots_found = 0;
  bool out = false;
  byte num;
  String strstr = String(str);
  
  Serial.println("ip_from_str");
  Serial.println(str);
  while(i < strlen(str) && dots_found < 3){
    if(str[i] == '.'){
      end_poss[dots_found] = i;
      dots_found++;
    }
    i++;
  }
  if(dots_found == 3){
    out = true;
    end_poss[3] = strlen(str);
    for(i = 0; i < 4; i++){
      num = String(strstr.substring(j, end_poss[i])).toInt();
      if(num == 0){
	out = false;
      }
      ip[i] = num;
      j = end_poss[i] + 1;
    }
  }
  if(out){
    Serial.print("IP: ");
    for(i=0; i<4; i++){
      Serial.print(ip[i]);
      if(i < 3){
	Serial.print(".");
      }
    }
    Serial.println();
  }
  return out;
}

void navkey_setup(){
  pinMode(IntPin, INPUT);
  Wire.begin();
  Serial.begin(115200);
  Serial.println("**** I2C navkey V2 basic example ****");
  /*
      INT_DATA= The register are considered integer.
      WRAP_ENABLE= The WRAP option is enabled
      DIRE_RIGHT= navkey right direction increase the value
      IPUP_ENABLE= INT pin have the pull-up enabled.
  */

  navkey.reset();
  navkey.begin(i2cNavKey::INT_DATA | i2cNavKey::WRAP_ENABLE | i2cNavKey::DIRE_RIGHT | i2cNavKey::IPUP_ENABLE);

  navkey.writeCounter((int32_t)0); /* Reset the counter value */
  navkey.writeMax((int32_t)255); /* Set the maximum threshold*/
  navkey.writeMin((int32_t)0); /* Set the minimum threshold */
  navkey.writeStep((int32_t)1); /* Set the step to 1*/

  navkey.writeDoublePushPeriod(30);  /*Set a period for the double push of 300ms */

  navkey.onUpPush = UP_Button_Pressed;
  navkey.onDownPush = DOWN_Button_Pressed;
  navkey.onRightPush = RIGHT_Button_Pressed;
  navkey.onLeftPush = LEFT_Button_Pressed;
  navkey.onCentralPush = CENTRAL_Button_Pressed;
  navkey.onCentralDoublePush = CENTRAL_Button_Double;
  navkey.onChange = Encoder_Rotate;

  navkey.autoconfigInterrupt(); /* Enable the interrupt with the attached callback */

  Serial.print("ID CODE: 0x");
  Serial.println(navkey.readIDCode(), HEX);
  Serial.print("Board Version: 0x");
  Serial.println(navkey.readVersion(), HEX);

}

void splash(){
  display_time(86400, 86400);
  my_show();
}
void led_setup(){
  FastLED.addLeds<LED_TYPE, DATA_PIN, CLK_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setDither(true);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
  FastLED.setBrightness(config.brightness);
  splash();
  for(int row = 0; row < MatrixHeight; row++){
    for(int col = 0; col < MatrixWidth; col++){
      leds[XY(col, row)] = CRGB::Blue;
      FastLED.show();
      delay(2);
    }
  }
}

void wifi_setup(){
  if(config.wifi_reset){
    config.wifi_reset = false;
    saveSettings();
    wifiManager.startConfigPortal("KLOK");
  }
  else{
    wifiManager.autoConnect("KLOK");
  }
  Serial.println("Yay connected!");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


/*********************************************************************************/
// Web Socket Server stuff
WebSocketsServer webSocket = WebSocketsServer(81);

void hexdump(const void *mem, uint32_t len, uint8_t cols) {
	const uint8_t* src = (const uint8_t*) mem;
	Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
	for(uint32_t i = 0; i < len; i++) {
		if(i % cols == 0) {
			Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
		}
		Serial.printf("%02X ", *src);
		src++;
	}
	Serial.printf("\n");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * ws_payload,
		    size_t length) {
  char topic_payload[length + 1];
  String str_topic_payload;
  int i;
  int start, stop;
  Serial.println("WebSocket Event!");
  switch(type) {
  case WStype_DISCONNECTED:
    Serial.printf("[%u] Disconnected!\n", num);
    break;
  case WStype_CONNECTED:
    {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], ws_payload);
      
      // send message to client
      webSocket.sendTXT(num, "Connected");
    }
    break;
  case WStype_TEXT:
    Serial.printf("[%u] get Text: %s\n", num, ws_payload);

    for(i=0; i < length; i++){
      topic_payload[i] = (char)ws_payload[i];
    }
    topic_payload[length] = 0;
    str_topic_payload = String(topic_payload);

    start = str_topic_payload.indexOf("//");
    stop = start + 2;
    if(start < 0){
      start = length;
      stop = length;
    }
    
    char topic[100];
    byte payload[100];
    for(i = 0; i < start; i++){
      topic[i] = topic_payload[i];
    }
    topic[start] = 0;

    for(i = 0; i < length - stop; i++){
      payload[i] = (byte)topic_payload[stop + i];
    }
    payload[length - stop] = 0;
    
    handle_msg(topic, payload, length - stop);

    if (strcmp(topic, "clockiot/get_displays") == 0) {
	// send display names to client
      String display_names = String("{\"displays\":[");
      for(int ii=0; ii < N_DISPLAY; ii++){
	//display_names = display_names + String("\"") + String(Displays[ii].name)  + String("\"");
	if(ii < N_DISPLAY - 1){
	  //display_names = display_names + String(",");
	}
      }
      display_names = display_names + String("],\"display_idx\":\"") + String(config.display_idx) + String("\"}");
      
      webSocket.sendTXT(num, display_names.c_str());
      Serial.println("Display names requested");
      }
    
    // send data to all connected clients
    // webSocket.broadcastTXT("message here");
    if (strcmp(topic, "clockiot/get_faceplates") == 0) {
	// send faceplate names to client
      String faceplate_names = String("{\"faceplates\":[");
      faceplate_names = faceplate_names + String("],\"faceplate_idx\":\"") + String(config.faceplate_idx) + String("\"}");
      
      webSocket.sendTXT(num, faceplate_names.c_str());
      Serial.println("Faceplate names requested");
      }
    
    // send data to all connected clients
    // webSocket.broadcastTXT("message here");
    break;
  case WStype_BIN:
    Serial.printf("[%u] get binary length: %u\n", num, length);
    hexdump(ws_payload, length, 16);
    
    // send message to client
    // webSocket.sendBIN(num, ws_payload, length);
    break;
  case WStype_ERROR:			
  case WStype_FRAGMENT_TEXT_START:
  case WStype_FRAGMENT_BIN_START:
  case WStype_FRAGMENT:
  case WStype_FRAGMENT_FIN:
    break;
  }
}

void websocket_setup(){
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket Setup!");
  
}

void factory_reset(){
  Serial.println("Factory RESET");
  config.timezone = 255; //?
  config.brightness = 6;
  config.display_idx = 255;
  config.use_wifi = 255;
  config.use_ip_timezone = 255;
  config.mqtt_ip[0] = 255;
  config.mqtt_ip[1] = 255;
  config.mqtt_ip[2] = 255;
  config.mqtt_ip[3] = 255;
  config.solid_color_rgb[0] = 0;
  config.solid_color_rgb[1] = 0;
  config.solid_color_rgb[2] = 255;
  config.second_color_rgb[0] = 0;
  config.second_color_rgb[1] = 255;
  config.second_color_rgb[2] = 0;
  config.third_color_rgb[0] = 255;
  config.third_color_rgb[1] = 0;
  config.third_color_rgb[2] = 0;
  config.flip_display = 255;
  config.last_tz_lookup = 0;
  config.use_ntp_time = true;
  config.wifi_reset = true;
  config.faceplate_idx = 255;
  config.factory_reset = false;
  saveSettings();
  print_config();

  Serial.println("Factory reset complete");
  ESP.restart();
}

void setup(){
  last_time = 0;
  
  Wire.begin();
  Serial.begin(115200);
  
  delay(200);
  Serial.println("setup() starting");

  EEPROM.begin(1024);
  loadSettings();
  print_config();

  navkey_setup();
  // logo
  if(config.brightness >= N_BRIGHTNESS){
    set_brightness(6);
  }
  led_setup(); // set up leds first so buttons can affect display if needed
  //factory_reset(); // manually call once, then recomment out and upload
  if(config.factory_reset){// do factory reset on first on
    factory_reset();
  }
  if(config.display_idx == 255){
    config.display_idx = 0;
    saveSettings();
  }
  
  if(config.use_wifi){
    wifi_setup();
  }
  if(config.use_wifi){
    if(config.use_ntp_time){
      ntp_clock.setup(&timeClient);
      ntp_clock.setOffset(config.timezone);
    }
    set_timezone_from_ip();
  }
  Serial.print("config.timezone: ");
  Serial.println(config.timezone);
  Serial.print("config.use_ip_timezone: ");
  Serial.println((bool)config.use_ip_timezone);

  websocket_setup();
  print_time();
  Serial.println("setup() complete");
}

uint32_t Now(){
  uint32_t out;
  
  if(config.use_wifi){
    if(config.use_ntp_time){
      out = ntp_clock.now();
      if(weekday(out) == 0){ // refresh utc offset sunday between 3 and 4 AM
	if(hour(out) == 3){
	  if(minute(out) > 1){ 
	    if(out - config.last_tz_lookup > 3601){
	      set_timezone_from_ip();
	    }
	  }
	}
      }
    }
    else{
      out = 42;
    }
  }
  else{
    out = 43;
  }
  return out;
}

void tobytes(const char *frm, byte *_to, int len){
  for(int ii = 0; ii < len; ii++){
    _to[ii] = frm[ii];
  }
}

void tochars(const char *frm, char *_to, int len){
  for(int ii = 0; ii < len; ii++){
    _to[ii] = frm[ii];
  }
  _to[len] = 0;
}

#define SERMAXLEN 100
char ser_msg[SERMAXLEN + 1];
uint8_t ser_msg_len = 0;
void serial_loop(){
  String ser_str, topic, payload;
  int start, stop;
  char topic_c_str[101];
  byte payload_bytes[100];
  
  //  msg format: topic//payload.  Example: "clockiot/timezone_offset//-14400"
  
  while(Serial.available() && ser_msg_len < SERMAXLEN){
    ser_msg[ser_msg_len++] = Serial.read();
  }
  if(ser_msg_len > 0){
    ser_str = String(ser_msg);
    start = ser_str.indexOf("clockiot");
    if(start >= 0){
      stop = ser_str.indexOf("//", start);
      if(stop < 0){
	stop = ser_str.length();
      }
      else{
      }
      topic = ser_str.substring(start, stop);
      if(stop == ser_str.length()){
      }
      else{
	stop += 2; // skip slashes "//"
      }
      payload = ser_str.substring(stop, ser_str.length());
      tochars(topic.c_str(), topic_c_str, topic.length());
      tobytes(payload.c_str(), payload_bytes, payload.length());
      handle_msg(topic_c_str, payload_bytes, payload.length());      
    }
  }
  // clear msg
  for(int ii=0; ii < ser_msg_len + 1; ii++){
    ser_msg[ii] = 0;
  }
  ser_msg_len = 0;
}

void navkey_loop() {
  uint8_t enc_cnt;
  if (digitalRead(IntPin) == LOW) {
    navkey.updateStatus();
  }
}

void  interact_loop(){
  if(force_timezone_from_ip){
    force_timezone_from_ip = false;
    set_timezone_from_ip();
  }
  if(config.use_wifi){
    webSocket.loop();
  }
  serial_loop();
  navkey_loop();
}

void fireNoise2(void);

void unix2hms(uint32_t unix, uint8_t *h, uint8_t *m, uint8_t *s){
  int spm = unix % 86400;
  int hh, mm, ss;
  hh = spm / 3600;
  mm = ((spm - hh * 3600) / 60); 
  ss = spm % 60;
  *h = hh;
  *m = mm;
  *s = ss;
}

void littleTime(uint32_t current_time, uint8_t start_x, uint8_t start_y, bool* mask){
  uint8_t hh, mm, ss;
  unix2hms(current_time, &hh, &mm, &ss);

  bool colen = (ss % 2) == 0;
  if(hh != 12){
    hh = hh % 12;
  }

  if(hh > 9){
    littleDigit(start_x + 0 * 5 - 1,  start_y, 1, mask);
  }
  littleDigit(start_x + 1 * 5 - 1,  start_y, hh % 10, mask);

  maskPixel(start_x + 9, start_y + 2, colen, mask);
  maskPixel(start_x + 9, start_y + 4, colen, mask);

  littleDigit(start_x + 2 * 5 + 1,  start_y, mm/10, mask);
  littleDigit(start_x + 3 * 5 + 1,  start_y, mm%10, mask);
}
void little3Code(char *code, uint8_t start_x, uint8_t start_y, bool* mask){
  
  littleChar(start_x +  0, start_y, code[0], mask);
  littleChar(start_x +  6, start_y, code[1], mask);
  littleChar(start_x + 12, start_y, code[2], mask);

}

void display_moon_phase(){
  uint8_t start_x =  4;

  // moon
  bool moon_mask[NUM_LEDS + 10];
  fillMask(moon_mask, false);
  moonPhase(start_x + 5, 45, moon_mask);
  paintMask(moon_mask, CRGB(0x80, 0xff, 0xff));// balanced white
}

void mask_big_time(uint8_t start_x, uint32_t current_time, bool *mask){
  uint8_t hh, mm, ss;
  unix2hms(current_time, &hh, &mm, &ss);
  bool colen = (ss % 2) == 0;
  //colen = true;

  if(hh != 12){
    hh = hh % 12;
  }

  if(hh > 9){
    bigDigit(start_x + 0 + 0,  0, (hh%12)/10, mask);
    bigDigit(start_x + 8 + 1,  0, (hh%12)%10, mask);
  }
  else{
    bigDigit(start_x + 8 - 4 + 1,  0, (hh%12)%10, mask);
  }
  bigDigit(start_x + 0 + 0, 20, mm/10, mask);
  bigDigit(start_x + 8 + 1, 20, mm%10, mask);
  bigDigit(start_x + 0 + 0, 40, ss/10, mask);
  bigDigit(start_x + 8 + 1, 40, ss%10, mask);
  maskPixel(start_x + 3    ,      17, colen, mask);
  maskPixel(start_x + 4    ,      17, colen, mask);
  maskPixel(start_x + 3    ,      18, colen, mask);
  maskPixel(start_x + 4    ,      18, colen, mask);
  maskPixel(start_x + 3 + 9,      17, colen, mask);
  maskPixel(start_x + 4 + 9,      17, colen, mask);
  maskPixel(start_x + 3 + 9,      18, colen, mask);
  maskPixel(start_x + 4 + 9,      18, colen, mask);
  maskPixel(start_x + 3    , 17 + 20, colen, mask);
  maskPixel(start_x + 4    , 17 + 20, colen, mask);
  maskPixel(start_x + 3    , 18 + 20, colen, mask);
  maskPixel(start_x + 4    , 18 + 20, colen, mask);
  maskPixel(start_x + 3 + 9, 17 + 20, colen, mask);
  maskPixel(start_x + 4 + 9, 17 + 20, colen, mask);
  maskPixel(start_x + 3 + 9, 18 + 20, colen, mask);
  maskPixel(start_x + 4 + 9, 18 + 20, colen, mask);
}

void mask_perpetual_calendar(bool *mask){
  for(int row = 0; row < MatrixWidth; row++){
    for(int col = 0; col < 4; col++){
      mask[XY(col, row)] = false;
    }
    mask[XY(23, row)] = false;
  }
}

void display_small_time(uint32_t last_time, uint32_t current_time){
  uint8_t start_x =  4;
  uint8_t start_y = current_time % 60;

  if(start_y > 29){
    start_y = 60 - start_y;
  }

  fill_solid(leds, NUM_LEDS, CRGB(config.solid_color_rgb[0],
  				  config.solid_color_rgb[1],
  				  config.solid_color_rgb[2]));
  fillMask(false, mask);
  //little3Code("   ", start_x + 3, start_y, mask);
  littleTime(current_time          , start_x, start_y, mask);
  apply_mask(mask);
}
void rainbow(){
  int i;
  uint8_t red, green, blue;
  
  for(uint8_t row=0; row < MatrixHeight; row++){
    for(uint8_t col=0; col < MatrixWidth; col++){
      i = XY(col, row);
      Wheel(((col * 10 + row * 10) + millis()/300) % 255, &red, &green, &blue);
      leds[i] = CRGB(red,green, blue);
    }
  }
}

void display_time(uint32_t last_time, uint32_t current_time){
  uint8_t word[3];                // will store start_x, start_y, length of word
  time_t spm;                     // seconds past midnight
  uint16_t time_inc;              // 5-minute time increment are we in

  spm = current_time % 86400;
  time_inc = spm / 300;
  //remove transitions for now
  if(time_inc != last_time_inc){
    CurrentDisplay_p->transition(last_time_inc, time_inc);
    last_time_inc = time_inc;
  }
  
  fillMask(mask, false);
  get_time_display(mask, time_inc);
  CurrentDisplay_p->background();
  apply_mask(mask);
  FastLED.show();
  return;
}

void loop(){
  uint8_t word[3];
  uint32_t current_time = Now();
  uint8_t month, day, dow;

  interact_loop();
  display_time(last_time, current_time);
  my_show();

  //print_time();
  //delay(1000);
  /*
  */
  if(config.use_wifi){
    if(config.use_ntp_time){
      if(ntp_clock.seconds() == 0 and millis() < 10000){
	Serial.print("Doomsday Time:");
	Serial.print(ntp_clock.year());
	Serial.print("/");
	Serial.print(ntp_clock.month());
	Serial.print("/");
	Serial.print(ntp_clock.day());
	Serial.print(" ");
	if(ntp_clock.hours() < 10)Serial.print('0');
	Serial.print(ntp_clock.hours());
	Serial.print(":");
	if(ntp_clock.minutes() < 10)Serial.print('0');
	Serial.print(ntp_clock.minutes());
	Serial.print(":");
	if(ntp_clock.seconds() < 10)Serial.print('0');
	Serial.println(ntp_clock.seconds());
      }
    }
  }
  last_time = current_time;
}
#include "Noise.h"
