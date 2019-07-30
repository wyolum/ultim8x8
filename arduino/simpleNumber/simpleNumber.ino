/* Take-a-number display 
 *  now serving...
 *  with websockets API for setting and incrementing the number.
 */
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
/*********************************************************************************/
// Web Socket Server stuff
WebSocketsServer webSocket = WebSocketsServer(81);
extern void websocketSetup();

#include <FastLED.h>
FASTLED_USING_NAMESPACE
#define ULTIM8x48
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
const char *password = "spamspamspamspam";

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
  websocket_setup();

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
  webSocket.loop();
  FastLED.setBrightness(brightness);
  fill_solid(leds, NUM_LEDS, solidColor);
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

/**********Websocket server stuff **************/
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * ws_payload, size_t length) {
  char topic_payload[length + 1];
  String str_topic_payload;
  int i;
  int start, stop;

  Serial.println("handling websocket event");
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
void handle_msg(char* topic, byte* payload, unsigned int length) {
  bool handled = false;
  char str_payload[length + 1];
  char *subtopic = topic + 12;

  // copy bytes to normal char array
  for(int i = 0; i < length; i++){
    str_payload[i] = payload[i];
  }
  str_payload[length] = 0;
  
  Serial.print("msg\n  subtopic:");
  Serial.println(subtopic);
  Serial.print("  payload:");
  Serial.println(str_payload);
  
  if(strcmp(subtopic, "brighter") == 0){
    Serial.println("Increment brigtness.");
    brighter();
  }
  else if(strcmp(subtopic, "dimmer") == 0){
    Serial.println("Decrement brigtness.");
    dimmer();
  }
  else if(strcmp(subtopic, "increment") == 0){
    Serial.println("Increment number");
    increment_number();
  }
  else if(strcmp(subtopic, "set_number") == 0){
    Serial.println("setNumber");
    set_number(String(str_payload).toInt());
  }
  else if(strcmp(subtopic, "reset_number") == 0){
    Serial.println("resetNumber");
    reset_number();
  }
  else if(strcmp(subtopic, "set_rgb") == 0 && length == 6){
    // payload: rrggbb lowercase html color code example "ff0000" is RED
    solid_color_rgb[0] = hh2dd((char*)payload);
    solid_color_rgb[1] = hh2dd((char*)payload + 2);
    solid_color_rgb[2] = hh2dd((char*)payload + 4);
    //force_update = true;
    //saveSettings();
  }
  else if(strcmp(subtopic, "notify") == 0){
    // payload: ascii notification
  }
}

void websocket_setup(){
  Serial.println("Setting up websocket server");
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);  
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
void saveSettings(){
  //TBD
}


