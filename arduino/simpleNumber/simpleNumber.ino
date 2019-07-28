/*
 * Copyright (c) 2015, Majenko Technologies
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Create a WiFi access point and provide a web server on it. */

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <FastLED.h>
FASTLED_USING_NAMESPACE
#define ULTIM8x48
#include <MatrixMaps.h>
#define DATA_PIN      13
#define CLK_PIN       14
#define LED_TYPE      APA102
#define COLOR_ORDER   BGR
#define NUM_LEDS      MatrixWidth * MatrixHeight

const bool MatrixSerpentineLayout = true;
#define MILLI_AMPS         1000     // IMPORTANT: set the max milli-Amps of your power supply (4A = 4000mA)
#define FRAMES_PER_SECOND  120 // here you can control the speed. With the Access Point / Web Server the animations run a bit slower.

int take_a_number = 57;
CRGB leds[NUM_LEDS];
const boolean FLIP_DISPLAY = true;
uint8_t brightness = 50;

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
/* Set these to your desired credentials. */
const char *ssid = "TakeANumber";
const char *password = "Password";

ESP8266WebServer server(80);

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot() {
	server.send(200, "text/html", "<h1>You are connected</h1>");
}

void setup() {
	delay(1000);
	Serial.begin(115200);
	Serial.println();
   FastLED.addLeds<LED_TYPE, DATA_PIN, CLK_PIN, COLOR_ORDER>(leds, NUM_LEDS); // for APA102 (Dotstar)
  FastLED.setDither(true);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

	Serial.print("Configuring access point...");
	/* You can remove the password parameter if you want the AP to be open. */
	WiFi.softAP(ssid, password);

	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);
	server.on("/", handleRoot);
	server.begin();
	Serial.println("HTTP server started");
}

void loop() {
  /* // test API
  //set_number((millis() / 1000) % 100);
  if (millis() > 1000 and millis() < 2000){
    increment_number();
  }
  else if(millis() < 10000){
    set_number((millis() / 1000) % 100);
  }
  else{
    reset_number();
  }
  */
  server.handleClient();
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  number();
  FastLED.show();
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);

}

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
  digit(8, t);
  digit(13, o);
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
