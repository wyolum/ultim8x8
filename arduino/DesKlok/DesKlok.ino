#include <Time.h>
#include <Wire.h>
#include <WiFiManager.h>
#include <FastLED.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <EEPROMAnything.h>
#include <NTPClient.h>
#include <I2CNavKey.h>
//#define ULTIM8x16 // DullesKlok
#include <ESP8266HTTPClient.h>

#include "textures.h"
#include "Noise.h"
#include "logic.h"
#include "get_time.h"

#include "config.h"
#include "MatrixMap.h"
#include "digits.h"
#include "fonts.h"
#include "moon_phases.h"
#include "english_v3.h"

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
const uint8_t N_DISPLAY = 9; // SNAKE DISABLED
const uint8_t N_BRIGHTNESS = 17;
const uint8_t BRIGHTNESSES[N_BRIGHTNESS] = {2,  3,  4,  5,  6,  7,  8, 10,
					    12, 14, 16, 19, 23, 27, 32, 38, 45};
bool setup_complete = false;
const int IntPin = 16; /* Definition of the interrupt pin*/


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
  uint8_t colorwheel_idx;
} config;

NTPClient timeClient(ntpUDP, "us.pool.ntp.org", 0, 60000); // do not use directly (only through ntp_clock)
NTPClock ntp_clock; // uses timeClient

uint16_t XY(int x, int y);
void set_timezone_offset(int32_t offset);
uint32_t Now();
void saveSettings();
void display_time(uint32_t last_time, uint32_t current_time);
void fillMask(bool* mask, bool b);
void fillMask(bool* mask, bool b, int start, int stop);
bool ip_from_str(char* str, byte* ip);

void Wheel(uint8_t WheelPos, uint8_t *red, uint8_t *green, uint8_t *blue);

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

bool force_update = false;

//const bool ON = true;
//const bool OFF = !ON;

// How many leds are in the strip?
const uint8_t N_BOARD = 2;

bool mask[NUM_LEDS];
bool wipe[NUM_LEDS];

WiFiClient espClient;

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

  //http.begin("https://timezoneapi.io/api/ip");// no longer works!
  //http.begin("https://ipapi.co/json");
  String url = String("http://www.wyolum.com/utc_offset/utc_offset.py") +
    String("?refresh=") + String(millis()) +
    String("&localip=") +
    String(WiFi.localIP()[0]) + String('.') + 
    String(WiFi.localIP()[1]) + String('.') + 
    String(WiFi.localIP()[2]) + String('.') + 
    String(WiFi.localIP()[3]) + String('&') +
    String("macaddress=") + WiFi.macAddress() + String('&') + 
    String("dev_type=DesKlok");
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

typedef struct{
  Init       init;           // called when display changes
  DisplayTime display_time;  // called in main loop to update time display (if needed)
  String     name;
  int        id;
} Display;

//--------------------------------------------------------------------------------
//uint32_t current_time;

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
  double theta;
  const uint8_t *phase;

  // some strange bug if get_moon_phase is called prior to setup completion
  if(setup_complete){
    theta = get_moon_phase(ntp_clock.gmt());
  }
  
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

void prev_display(){
    config.display_idx = (config.display_idx + N_DISPLAY - 1) % N_DISPLAY;
    saveSettings();
    Serial.print("Display incremented to: ");
    Serial.print(config.display_idx);
}

void next_display(){
    config.display_idx = (config.display_idx + 1) % N_DISPLAY;
    saveSettings();
    Serial.print("Display incremented to: ");
    Serial.print(config.display_idx);
}

void toggle_sleep(){
  sleeping = !sleeping;
  if(sleeping){
    Serial.println("Going to sleep.");
  }
  else{
    Serial.println("Waking up.");
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

void splash(){
  // display_time(86400, 86400);

  fillMask(false, mask);
  
  //fill_solid(leds, NUM_LEDS, CRGB::Blue);
  fill_solid(leds, NUM_LEDS, CRGB(config.solid_color_rgb[0],
  				  config.solid_color_rgb[1],
  				  config.solid_color_rgb[2]));

  int start_x = 3;
  int start_y = 8;
  littleChar(start_x +  0, start_y, 'W', mask);
  littleChar(start_x +  6, start_y, 'Y', mask);
  littleChar(start_x + 12, start_y, 'O', mask);
  littleChar(start_x + 0, start_y + 7 * 1, 'B', mask);
  littleChar(start_x + 5, start_y + 7 * 2, 'E', mask);
  littleChar(start_x + 10, start_y + 7 * 3, 'A', mask);
  littleChar(start_x + 15, start_y + 7 * 4, 'N', mask);
  my_show();
}

void led_setup(){
  FastLED.addLeds<LED_TYPE, DATA_PIN, CLK_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setDither(true);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
  FastLED.setBrightness(BRIGHTNESSES[config.brightness]);
  splash();
}

void wifi_setup(){
  //wifiManager.resetSettings();
  if(config.wifi_reset){
    config.wifi_reset = false;
    saveSettings();
    wifiManager.startConfigPortal("DesKlok");
  }
  else{
    wifiManager.autoConnect("DesKlok");
  }
  Serial.println("Yay connected!");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


/*********************************************************************************/
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
  config.colorwheel_idx = 0;
  config.factory_reset = false;
  saveSettings();

  Serial.println("Factory reset complete");
  ESP.restart();
}

void set_brightness(uint8_t brightness){
  if(brightness < N_BRIGHTNESS){
    config.brightness = brightness;
    FastLED.setBrightness(BRIGHTNESSES[config.brightness]);
    Serial.print("Adjust brightness to ");
    Serial.println(config.brightness);
    saveSettings();
  }
}

void setup(){
  my_leds = &leds[0];
  last_time = 0;
  
  Wire.begin();
  Serial.begin(115200);
  
  delay(200);
  Serial.println("setup() starting");

  EEPROM.begin(1024);
  loadSettings();
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

  
  Serial.println("setup() complete");
  setup_complete = true;
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
      out = 42; // debug display
    }
  }
  else{
    out = 43; // debug display
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


void getword(int i, uint8_t* out){
  out[0] = pgm_read_byte(WORDS + 3 * i + 1);
  out[1] = pgm_read_byte(WORDS + 3 * i + 2);
  out[2] = pgm_read_byte(WORDS + 3 * i + 3);
}
void get_time_display(bool* mask, int i){
  uint8_t bits;     // holds the on off state for 8 words at a time
  uint8_t word[3];  // start columm, start row, length of the current word
  uint8_t n_byte_per_display = 4;         // number of bytes used for each 5 minunte time incriment

  for(uint8_t j = 0; j < n_byte_per_display; j++){ // j is a byte index 
    
    // read the state for the next set of 8 words
    bits = pgm_read_byte(DISPLAYS + 1 + (i * n_byte_per_display) + j);
    Serial.print(bits);
    Serial.print(" ");
    for(uint8_t k = 0; k < 8; k++){                     // k is a bit index
      if((bits >> k) & 1){                              // check to see if word is on or off
	getword(j * 8 + k, word);                       // if on, read location and length
	for(int m=word[0]; m < word[0] + word[2]; m++){ // and display it
	  setPixelMask(mask, word[1], m, true);
	}
      }
    }
    Serial.println();
  } 
}

void display_time(uint32_t last_time, uint32_t current_time){
  int spm = current_time % 86400;
  int time_inc = spm / 300;
  fillMask(mask, false);
  get_time_display(mask, time_inc);
  for(int i=0; i< NUM_LEDS; i++){
    if(mask[i]){
      leds[i] = CRGB::Blue;
    }
    else{
      leds[i] = CRGB::Black;
    }
  }
}

void loop(){
  uint8_t word[3];
  uint32_t current_time = Now();
  uint8_t current_month, current_day, current_dow;
  
  display_time(last_time, current_time);
  my_show();
  return;
}
