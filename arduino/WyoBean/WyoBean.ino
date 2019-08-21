#include <Time.h>
#include <Wire.h>
#include <WiFiManager.h>
#include <FastLED.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <EEPROMAnything.h>
#include <NTPClient.h>
#include <WebSocketsServer.h>

//#define ULTIM8x16 // DullesKlok
#include <ESP8266HTTPClient.h>

#include "textures.h"
#include "logic.h"
#include "get_time.h"

#include "config.h"
#include "MatrixMap.h"
#include "digits.h"

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
  bool use_ntp_time;
  bool wifi_reset;
  uint8_t faceplate_idx;
} config;

void maskPixel(byte row, byte col, bool val, bool *mask);
void maskPixel(byte row, byte col, bool val, bool *mask){
  uint16_t pos = XY(row, col);
  
  if(0 <= pos && pos < NUM_LEDS){
    mask[pos] = val;
  }
}

void print_config(){
  Serial.println("config:");
  Serial.print("    timezone:"); Serial.println(config.timezone);
  Serial.print("    brightness:"); Serial.println(config.brightness);
  Serial.print("    display_idx:"); Serial.println(config.display_idx);
  Serial.print("    factory_reset:"); Serial.println(config.factory_reset);
  Serial.print("    use_wifi:"); Serial.println(config.use_wifi);
  Serial.print("    use_ip_timezone:"); Serial.println(config.use_ip_timezone);
  Serial.print("    mqtt_ip:");
  for(int ii = 0; ii < 4; ii++){
    if (ii > 0){
      Serial.print(".");
    }
    Serial.print(config.mqtt_ip[ii]);
  }
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
  Serial.println();
  Serial.print("    use_ntp_time:"); Serial.println(config.use_ntp_time);
  Serial.print("    wifi_reset:"); Serial.println(config.wifi_reset);
  Serial.print("    faceplate_idx:"); Serial.println(config.faceplate_idx);
}
bool force_update = false;

//const bool ON = true;
//const bool OFF = !ON;

// How many leds are in the strip?
const uint8_t N_BOARD = 2;

bool mask[NUM_LEDS];
bool wipe[NUM_LEDS];
CRGB leds[NUM_LEDS];

WiFiClient espClient;
PubSubClient mqtt_client(espClient);
void  interact_loop();

#define DATA_PIN     MOSI
#define CLK_PIN      SCK

#define COLOR_ORDER BGR
#define LED_TYPE APA102
#define MILLI_AMPS 500  // IMPORTANT: set the max milli-Amps of your power supply (4A = 4000mA)

uint32_t last_time;

//********************************************************************************
// Displays

void my_show(){
  FastLED.show();
  interact_loop();
}

typedef void (*Init)();
typedef void (*DisplayTime)(uint32_t last_tm, uint32_t tm);

NTPClock ntp_clock;

WiFiManager wifiManager;
WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "us.pool.ntp.org", 0, 60000);

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
  String url = String("https://www.wyolum.com/utc_offset/utc_offset.py") +
    String("?refresh=") + String(millis()) +
    String("&localip=") +
    String(WiFi.localIP()[0]) + String('.') + 
    String(WiFi.localIP()[1]) + String('.') + 
    String(WiFi.localIP()[2]) + String('.') + 
    String(WiFi.localIP()[3]) + String('&') +
    String("macaddress=") + WiFi.macAddress() + String('&') + 
    String("dev_type=ClockIOT");
  url = String("https://www.wyolum.com");
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

void wipe_around(bool val){
  float dtheta = 31.4 / 180;
  float theta = -3.14 - dtheta;
  int row, col;
  bool tmp[NUM_LEDS];

  int cx = random(0, MatrixWidth-1);
  int cy = random(0, MatrixHeight-1);
  cx = 8;
  cy = 4;
  
  fillMask(wipe, !val); // 
  while (theta < 3.14 + dtheta){
    for(row=0; row < MatrixHeight; row++){
      for(col=0; col < MatrixWidth; col++){
	if(atan2(row - cy, col - cx) < theta){
	  setPixelMask(wipe, row, col, val); // set wedge to val
	}
      }
    }
    if(val){
      logical_or(NUM_LEDS, wipe, mask, tmp);
    }
    else{
      logical_and(NUM_LEDS, wipe, mask, tmp);
    }
    apply_mask(tmp);
    my_show();
    theta += dtheta;
  }
  
}

typedef struct{
  Init       init;           // called when display changes
  DisplayTime display_time;  // called in main loop to update time display (if needed)
  String     name;
  int        id;
} Display;

void Plain_init(){
  return;
  while(1){
    fill_blue();
    fillMask(mask, ON);
    wipe_around(ON);
    Serial.println("HERE");
    delay(500);

    wipe_around(OFF);
    my_show();
    delay(500);
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

void rainbow_cycle(int count){
  int i, dx, dy;
  CHSV hsv;
  float dist;
  
  hsv.hue = 0;
  hsv.val = 255;
  hsv.sat = 240;

  for( int row = 0; row < MatrixHeight; row++) {
    for( int col = 0; col < MatrixWidth; col++) {
      // dx, dy, dist used for radial pattern, not used here
      dy = (row - 4) * 2;
      dx = col - 8;
      dist = sqrt(dx * dx + dy * dy);
      i = XY(col, row);
      //hsv.hue =  ((int)(dist * 16) - count) % 256;
      hsv.hue =  (count + (MatrixWidth * row + col) * 2) % 256;
      leds[i] = hsv;
    }
  }
}

void rainbow_fast() {
  uint32_t current_time = Now();
  int count = millis() / 100;
  
  rainbow_cycle(millis()/25);
  // Show the leds (only one of which is set to white, from above)
  //delay(100);
}

void rainbow_slow() {
  uint32_t current_time = Now();
  int count = ((current_time % 300) * 255) / 300;
  rainbow_cycle(count);
}


uint8_t logo_rgb[] = {
  0x11,0x00,0x29,0x00,0x25,0x00,0x23,0x00,0x25,0x00,0x29,0x00,0x31,0x00,0xe0,0x01,
  0x00,0x03,0x80,0x04,0x80,0x04,0x80,0x04,0x80,0x04,0x00,0x03,0x00,0x00,0x00,0x00,
  0x11,0x00,0x09,0x88,0x05,0x48,0x03,0x28,0x05,0x18,0x09,0x28,0x11,0x48,0x00,0x88
};

// Common Interface for buttons and MQTT
void set_brightness(uint8_t brightness){
  if(brightness < 256){
    config.brightness = brightness;
    FastLED.setBrightness(config.brightness);
    Serial.print("Adjust brightness to ");
    Serial.println(config.brightness);
    saveSettings();
  }
}

void adjust_brightness(int delta){
  int new_val = delta + config.brightness;
  if(delta != 0){
    if(0 < new_val && new_val < 256){
      set_brightness(new_val);
    }
  }
}

void dimmer(){
  byte b;
  b = config.brightness;
  if(b == 255){
    b = 128;
  }
  else if(b > 3){
    b /= 2;
  }
  else if (b == 3){
    b = 2;
  }
  else{
    b = 2;
  }
  set_brightness(b);
}
void brighter(){
  byte b;
  b = config.brightness;
  if(b >= 128){
    b = 255;
  }
  else if(b < 128){
    b *= 2;
  }
  else{
    b = 128;
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

void applyMask(bool* mask){
  uint16_t b, k;
  for(uint16_t i=0; i < NUM_LEDS; i++){
    if(!mask[i]){
      leds[i] = CRGB::Black;
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

bool rotate_display = true;
uint16_t XY(int x, int y){
  uint16_t out;
  if(rotate_display){
    int tmp = x;
    x = y;
    y = MatrixHeight - tmp;
  }
  
  if(0 <= x && x < MatrixWidth &&
     0 <= y && y < MatrixHeight){
    out = MatrixMap[y][x];
  }
  else{
    out = 0;
  }
  return out;
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
  else if(strcmp(subtopic, "next_display") == 0){
    Serial.println("Increment display, not supported.");
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
    if(config.flip_display){
      config.flip_display = false;
    }
    else{
      config.flip_display = true;
    }      
    Serial.print("Flip Display:");
    Serial.println(config.flip_display);
    force_update = true;
    saveSettings();
  }
  else if(strcmp(subtopic, "mqtt_ip") == 0){
    Serial.println("Update mqtt_ip address.");
    byte tmp_ip[4];
    if(ip_from_str(str_payload, tmp_ip)){
      for(int i=0; i<4; i++){
	config.mqtt_ip[i] = tmp_ip[i];
      }
      saveSettings();
      mqtt_setup();
    }
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
    force_update = true;
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

void handle_mqtt_msg(char* topic, byte* payload, unsigned int length){
  handle_msg(topic, payload, length);
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

void mqtt_subscribe(){
  mqtt_client.subscribe("clockiot/#");
}

uint32_t next_mqtt_attempt = 0;

bool mqtt_connect(){
  String str;
  String unique_id = String("ClockIOT") + String(WiFi.macAddress());

  if(!mqtt_client.connected() && next_mqtt_attempt < millis()){
    if(mqtt_client.connect(unique_id.c_str())){
      Serial.println("mqtt connected");
      Serial.println(unique_id);
      // Once connected, publish an announcement...
      // ... and resubscribe
      mqtt_subscribe();
    }
  }
  uint32_t n = millis();
  
  next_mqtt_attempt = n + 5000;
  
  return mqtt_client.connected();
}

void mqtt_setup(){
  //uint8_t server[4] = {192, 168, 1, 159};
  //uint8_t server[4] = {10, 10, 10, 2};
  mqtt_client.setServer(config.mqtt_ip, 1883);
  mqtt_client.setCallback(handle_mqtt_msg);
  mqtt_connect();
  Serial.println("USE MQTT");
}

void splash(){
  display_time(0, 0);
  my_show();
  delay(1000);
}
void led_setup(){
  FastLED.addLeds<LED_TYPE, DATA_PIN, CLK_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setDither(true);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
  FastLED.setBrightness(config.brightness);
  splash();
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

void factory_reset(){
  Serial.println("Factory RESET");
  config.timezone = 255; //?
  config.brightness = 8;
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
  config.flip_display = 255;
  config.last_tz_lookup = 0;
  config.use_ntp_time = true;
  config.wifi_reset = true;
  config.faceplate_idx = 255;
  config.factory_reset = false;
  saveSettings();
  print_config();
  delay(1000);

  Serial.println("Factory reset complete");
  ESP.restart();
}

bool use_mqtt(){
  bool out = false;
  for(int i=0; i<4; i++){
    if(config.mqtt_ip[i] != 255){
      out = true;
      break;
    }
  }
  return out;
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

  // logo
  if( config.brightness == 0 || config.brightness == 255){
    set_brightness(8);
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
  if(use_mqtt()){
    mqtt_setup();
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
void serial_loop(){/// allow same msgs as mqtt
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

void  interact_loop(){
  if(force_timezone_from_ip){
    force_timezone_from_ip = false;
    set_timezone_from_ip();
  }
  if (use_mqtt()){
    mqtt_client.loop();
  }
  if(config.use_wifi){
  }
  serial_loop();
}

void fireNoise2(void);

void display_time(uint32_t last_time, uint32_t current_time){
  int hh, mm, ss;
  int spm = current_time % 86400;
  hh = spm / 3600;
  mm = ((spm - hh * 3600) / 60); 
  ss = spm % 60;
  if(false){
    Serial.print("hh:");
    Serial.print(hh);
    Serial.print(" mm:");
    Serial.print(mm);
    Serial.print(" ss:");
    Serial.println(ss);
  }
  
  bool colen = (ss % 2) == 0;
  
  //fill_solid(leds, NUM_LEDS, CRGB::Blue);
  //fireNoise2();
  fill_solid(leds, NUM_LEDS, CRGB(config.solid_color_rgb[0],
  				  config.solid_color_rgb[1],
  				  config.solid_color_rgb[2]));
  /*
  for(int i=0; i < 3; i++){
    Serial.print(config.solid_color_rgb[i]);
    Serial.print(" ");
  }
  Serial.println();
  */
  bigDigit(4 + 0 + 0,  0, hh/10, mask);
  bigDigit(4 + 8 + 1,  0, hh%10, mask);
  bigDigit(4 + 0 + 0, 20, mm/10, mask);
  bigDigit(4 + 8 + 1, 20, mm%10, mask);
  bigDigit(4 + 0 + 0, 40, ss/10, mask);
  bigDigit(4 + 8 + 1, 40, ss%10, mask);
  maskPixel(         7,      17, colen, mask);
  maskPixel(         8,      17, colen, mask);
  maskPixel(         7,      18, colen, mask);
  maskPixel(         8,      18, colen, mask);
  maskPixel(     7 + 9,      17, colen, mask);
  maskPixel(     8 + 9,      17, colen, mask);
  maskPixel(     7 + 9,      18, colen, mask);
  maskPixel(     8 + 9,      18, colen, mask);
  maskPixel(         7, 17 + 20, colen, mask);
  maskPixel(         8, 17 + 20, colen, mask);
  maskPixel(         7, 18 + 20, colen, mask);
  maskPixel(         8, 18 + 20, colen, mask);
  maskPixel(     7 + 9, 17 + 20, colen, mask);
  maskPixel(     8 + 9, 17 + 20, colen, mask);
  maskPixel(     7 + 9, 18 + 20, colen, mask);
  maskPixel(     8 + 9, 18 + 20, colen, mask);
  applyMask(mask);

}


void loop(){
  uint8_t word[3];
  uint32_t current_time = Now();

  display_time(last_time, current_time);
  my_show();

  /*
  Serial.print("NTP Time:");
  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.print(timeClient.getMinutes());
  Serial.print(":");
  Serial.print(timeClient.getSeconds());
  Serial.println("");
  
  delay(1000);
  */
  if(config.use_wifi){
    if(config.use_ntp_time){
      if(ntp_clock.seconds() == 0 and millis() < 1){
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
