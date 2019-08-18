/*
 * WebSocketClient.ino
 *
 *  Created on: 24.05.2015
 *
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <Hash.h>
#include <FastLED.h>
FASTLED_USING_NAMESPACE
#define ULTIM8x24
#include <MatrixMaps.h>
#define DATA_PIN      13
#define CLK_PIN       14
#define LED_TYPE      APA102
#define COLOR_ORDER   BGR
#define NUM_LEDS      MatrixWidth * MatrixHeight
CRGB solidColor = CRGB::Blue;
uint8_t solid_color_rgb[3];
const bool MatrixSerpentineLayout = true;
#define MILLI_AMPS         1000     // IMPORTANT: set the max milli-Amps of your power supply (4A = 4000mA)
#define FRAMES_PER_SECOND  120 // here you can control the speed. With the Access Point / Web Server the animations run a bit slower.

int take_a_number = 42;
CRGB leds[NUM_LEDS];
const boolean FLIP_DISPLAY = true;
uint8_t brightness = 50;

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

const byte digits4x8[8*10] = {
  0x06,0x09,0x09,0x09,0x09,0x09,0x09,0x06, // 0
  0x04,0x06,0x04,0x04,0x04,0x04,0x04,0x0e, // 1
  0x06,0x09,0x08,0x08,0x04,0x02,0x01,0x0f, // 2
  0x06,0x09,0x08,0x04,0x08,0x08,0x09,0x06, // 3
  0x04,0x05,0x05,0x05,0x0f,0x04,0x04,0x04, // 4
  0x0f,0x01,0x01,0x07,0x08,0x08,0x09,0x06, // 5
  0x06,0x09,0x01,0x07,0x09,0x09,0x09,0x06, // 6
  0x0f,0x08,0x08,0x04,0x02,0x01,0x01,0x01, // 7
  0x06,0x09,0x09,0x06,0x09,0x09,0x09,0x06, // 8
  0x06,0x09,0x09,0x09,0x0e,0x08,0x09,0x06, // 9
};

bool mask[NUM_LEDS];

void togglePixelMask(uint8_t row, uint8_t col, bool b){
  mask[XY(col, row)] = ! mask[XY(col, row)];
}

void setPixelMask(uint8_t row, uint8_t col, bool b){
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

#define USE_SERIAL Serial

bool broadcast_number(){
  char base[] = "takeanumber/set_number//";
  char *digits = "XX";
  digits = itoa(take_a_number % 100, digits, 10);
  char *msg = strcat(base, digits);
  msg[24 + 2] = 0;
  //USE_SERIAL.println(msg);
  return webSocket.sendTXT(msg);
}
// start API
void set_number(int number){
  take_a_number = number;
}
int get_number(){
  return take_a_number;
}
void increment_number(){
  set_number(take_a_number + 1);
}
void reset_number(){
  set_number(0);
}
void brighter(){
  if(brightness < 128){
    brightness *= 2;
  }
  else{
    brightness = 255;
  }
}
void dimmer(){
  if(brightness > 1){
    brightness /= 2;
  }
  else{
    brightness = 1;
  }
}
// end API

uint16_t XY( uint8_t x, uint8_t y){
  if(FLIP_DISPLAY){
    x = MatrixWidth - x - 1;
    y = MatrixHeight - y - 1;
  }
  uint16_t out = 0;
  if(x < MatrixWidth && y < MatrixHeight){
    out = MatrixMap[y][x];
  }
  return out;
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

  switch(type) {
  case WStype_DISCONNECTED:
    USE_SERIAL.printf("[WSc] Disconnected!\n");
    break;
  case WStype_CONNECTED: {
    USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);

    // send message to server when Connected
    webSocket.sendTXT("Connected");
  }
    break;
  case WStype_TEXT:
    USE_SERIAL.printf("[WSc] get text: %s\n", payload);

    // send message to server
    // webSocket.sendTXT("message here");
    break;
  case WStype_BIN:
    USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
    hexdump(payload, length);

    // send data to server
    // webSocket.sendBIN(payload, length);
    break;
  }

}

void setup() {
  // USE_SERIAL.begin(921600);
  USE_SERIAL.begin(115200);
  
  //Serial.setDebugOutput(true);
  USE_SERIAL.setDebugOutput(true);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for(uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  WiFiMulti.addAP("TakeANumber", "spamspamspamspam");

  //WiFi.disconnect();
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  // server address, port and URL
  webSocket.begin("192.168.4.1", 81, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);

  // use HTTP Basic Authorization this is optional remove if not needed
  //webSocket.setAuthorization("user", "Password");

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);

  FastLED.addLeds<LED_TYPE, DATA_PIN, CLK_PIN, COLOR_ORDER>(leds, NUM_LEDS); // for APA102 (Dotstar)
  FastLED.setDither(true);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();  
}

// preceed with a call to fillMask(false);
// set mask to true where digit should light
void digit(byte start, byte d){
  byte row, col;
  for(col = 0; col < 4; col++){
    for(row = 0; row < 8; row++){
      if((digits4x8[d * 8 + row] >> col) & 1){
	setPixelMask(row, col + start, true);
	setPixelMask(row, col + start + 24, true);
	//setPixelMask(row + 8, col + start, true);
	setPixelMask(row + 16, col + start, true);
      }
      else{
      }
    }
  }
}

void displayNumber(uint32_t number){
  int h, t, o;
  int hto = number % 1000;

  h = (number / 100) % 10;
  t = (number / 10) % 10;
  o = number % 10;
  //digit( 6, h);
  digit(8 + 4, t);
  digit(13 + 4, o);
}
// set mask to all masked (b=false) or all unmasked (b = true)
void fillMask(bool b){
  fillMask(b, 0, NUM_LEDS);
}

void fillMask(bool b, int start, int stop){
  for(int i = start; i < stop && i < NUM_LEDS; i++){
    mask[i] = b;
  }
}

// turn off leds where mask[i] = false
void apply_mask(){
  uint16_t b, k;
  for(uint16_t i=0; i < NUM_LEDS; i++){
    if(!mask[i]){
      leds[i] = CRGB::Black;
    }
    else{
      // leds[i] = CRGB::Green;
    }
  }
}

void number(){
  fillMask(false);
  displayNumber(take_a_number);
  apply_mask();
}

int wait_ms = 200;
int accel = 10;
void loop() {
  webSocket.loop();
  /*
    webSocket.sendTXT("takeanumber/increment");
    if(wait_ms > 120){
    wait_ms-= accel;
    accel += 1;
    delay(wait_ms);
    }*/
  fill_solid(leds, NUM_LEDS, solidColor);
  number();
  leds[0] = CRGB::Black;

  FastLED.show();
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);
  take_a_number++;
  if(broadcast_number()){
    USE_SERIAL.println("Success");
  }
  USE_SERIAL.println(take_a_number);
  delay(1000);
}
