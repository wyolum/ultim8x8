/*
   ESP8266 + FastLED + IR Remote: https://github.com/jasoncoon/esp8266-fastled-webserver
   Copyright (C) 2015-2016 Jason Coon

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <WiFiUdp.h>
#include <FastLED.h>
#include <credentials.h>
/* ---- credentials.h ----
char ssid[] = "XXXXXXXXXX";  // your network SSID (name)
char pass[] = "YYYYYYYYYY";  // your network password
*/
FASTLED_USING_NAMESPACE

extern "C" {
#include "user_interface.h"
}

//#define ULTIM24x24
//#define ULTIM16x56
#define ULTIM8x48
#include <MatrixMaps.h>

#include <ESP8266WiFi.h>
//#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WebSocketsServer.h>
#include <FS.h>
#include <EEPROM.h>
//#include <IRremoteESP8266.h>
#include "GradientPalettes.h"


#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#include "Field.h"

#define HOSTNAME "ESP8266-Feather-" ///< Hostname. The setup function adds the Chip ID at the end.

//#define RECV_PIN D4
//IRrecv irReceiver(RECV_PIN);

//#include "Commands.h"

const bool apMode = false;

// AP mode pass
const char WiFiAPPSK[] = "";

ESP8266WebServer webServer(80);
WebSocketsServer webSocketsServer = WebSocketsServer(81);
ESP8266HTTPUpdateServer httpUpdateServer;

const unsigned int localPort = 2390;     // local port to listen for UDP packets
WiFiUDP udp;

#include "FSBrowser.h"

#define DATA_PIN      13
#define CLK_PIN       14
#define LED_TYPE      APA102
#define COLOR_ORDER   BGR
#define NUM_LEDS      MatrixWidth * MatrixHeight

const bool MatrixSerpentineLayout = true;

#define MILLI_AMPS         1000     // IMPORTANT: set the max milli-Amps of your power supply (4A = 4000mA)
#define FRAMES_PER_SECOND  120 // here you can control the speed. With the Access Point / Web Server the animations run a bit slower.

CRGB leds[NUM_LEDS];

int32_t last_lag_ms = 0;
uint32_t local_hack = 0;
uint32_t local_hack_ms = 0;
uint32_t local_hack_us = 0;
uint16_t ms_per_second = 1000;   // may change to correct clock drift
uint32_t current_time;
uint32_t last_update_time = 0;// larger than initialization time
bool clock_initialized = false;
const uint32_t NTP_UPDATE_INTERVAL = 100; // Seconds

const uint8_t brightnessCount = 5;
uint8_t brightnessMap[brightnessCount] = { 16, 32, 64, 128, 255 };
uint8_t brightnessIndex = 0;

// ten seconds per color palette makes a good demo
// 20-120 is better for deployment
uint8_t secondsPerPalette = 10;

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100
uint8_t cooling = 49;

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
uint8_t sparking = 60;

uint8_t speed = 30;

///////////////////////////////////////////////////////////////////////

// Forward declarations of an array of cpt-city gradient palettes, and
// a count of how many there are.  The actual color palette definitions
// are at the bottom of this file.
extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];

uint8_t gCurrentPaletteNumber = 0;

CRGBPalette16 gCurrentPalette( CRGB::Black);
CRGBPalette16 gTargetPalette( gGradientPalettes[0] );

CRGBPalette16 IceColors_p = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);

uint8_t currentPatternIndex = 0; // Index number of which pattern is current
uint8_t autoplay = 0;

uint8_t autoplayDuration = 10;
unsigned long autoPlayTimeout = 0;

uint8_t currentPaletteIndex = 0;

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

CRGB solidColor = CRGB::Blue;

const boolean FLIP_DISPLAY = true;
uint16_t XY( uint8_t x, uint8_t y)
{
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

// scale the brightness of all pixels down
void dimAll(byte value)
{
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(value);
  }
}

typedef struct {
  CRGBPalette16 palette;
  String name;
} PaletteAndName;
typedef PaletteAndName PaletteAndNameList[];

const CRGBPalette16 palettes[] = {
  RainbowColors_p,
  RainbowStripeColors_p,
  CloudColors_p,
  LavaColors_p,
  OceanColors_p,
  ForestColors_p,
  PartyColors_p,
  HeatColors_p
};

const uint8_t paletteCount = ARRAY_SIZE(palettes);

const String paletteNames[paletteCount] = {
  "Rainbow",
  "Rainbow Stripe",
  "Cloud",
  "Lava",
  "Ocean",
  "Forest",
  "Party",
  "Heat",
};

typedef void (*Pattern)();
typedef Pattern PatternList[];
typedef struct {
  Pattern pattern;
  String name;
} PatternAndName;
typedef PatternAndName PatternAndNameList[];

#include "Twinkles.h"
#include "TwinkleFOX.h"
#include "Map.h"
#include "Noise.h"
#include "sunrise.h"

void clock();
// List of patterns to cycle through.  Each is defined as a separate function below.

PatternAndNameList patterns = {
  { sunriseStatic,          "Sunrise"},
  { sunriseFlicker,         "Sunrise Flicker"},
  { sunriseWavesDiagonal,   "Sunrise Diagonal"},
  { sunriseWavesVertical,   "Sunrise Waves Vertical"},
  { sunriseWavesHorizontal, "Sunrise Waves Horizontal"},
  { sunriseWavesRotating,   "Sunrise Waves Rotating"},
  { pride,                  "Pride" },
  { pride2,                 "Pride 2" },
  { colorWaves,             "Color Waves" },
  { colorWaves2,            "Color Waves 2" },

  /*
  { cubeTest,       "Cube XYZ Test" },
  
  { cubeXPalette,   "Cube X Palette" },
  { cubeYPalette,   "Cube Y Palette" },
  { cubeZPalette,   "Cube Z Palette" },
  
  { cubeXYPalette,  "Cube XY Palette" },
  { cubeXZPalette,  "Cube XZ Palette" },
  { cubeYZPalette,  "Cube YZ Palette" },
  { cubeXYZPalette, "Cube XYZ Palette" },

  { cubeXGradientPalette,   "Cube X Gradient Palette" },
  { cubeYGradientPalette,   "Cube Y Gradient Palette" },
  { cubeZGradientPalette,   "Cube Z Gradient Palette" },
  
  { cubeXYGradientPalette,  "Cube XY Gradient Palette" },
  { cubeXZGradientPalette,  "Cube XZ Gradient Palette" },
  { cubeYZGradientPalette,  "Cube YZ Gradient Palette" },
  { cubeXYZGradientPalette, "Cube XYZ Gradient Palette" },
  { fireNoise3d, "Fire Noise 3D" },
  { fireNoise23d, "Fire Noise 2 3D" },
  { lavaNoise3d, "Lava Noise 3D" },
  { rainbowNoise3d, "Rainbow Noise 3D" },
  { rainbowStripeNoise3d, "Rainbow Stripe Noise 3D" },
  { partyNoise3d, "Party Noise 3D" },
  { forestNoise3d, "Forest Noise 3D" },
  { cloudNoise3d, "Cloud Noise 3D" },
  { oceanNoise3d, "Ocean Noise 3D" },
  { blackAndWhiteNoise3d, "Black & White Noise 3D" },
  { blackAndBlueNoise3d, "Black & Blue Noise 3D" },
  */
  
  // 3d noise patterns
  { xyMatrixTest,           "Matrix Test" },

  { verticalPalette,           "Vertical Palette" },
  { diagonalPalette,           "Diagonal Palette" },
  { horizontalPalette,         "Horizontal Palette" },

  { verticalGradientPalette,   "Vertical Gradient Palette" },
  { diagonalGradientPalette,   "Diagonal Gradient Palette" },
  { horizontalGradientPalette, "Horizontal Gradient Palette" },

  // noise patterns
  { fireNoise, "Fire Noise" },
  { fireNoise2, "Fire Noise 2" },
  { lavaNoise, "Lava Noise" },
  { rainbowNoise, "Rainbow Noise" },
  { rainbowStripeNoise, "Rainbow Stripe Noise" },
  { partyNoise, "Party Noise" },
  { forestNoise, "Forest Noise" },
  { cloudNoise, "Cloud Noise" },
  { oceanNoise, "Ocean Noise" },
  { blackAndWhiteNoise, "Black & White Noise" },
  { blackAndBlueNoise, "Black & Blue Noise" },

  // twinkle patterns
  { rainbowTwinkles,        "Rainbow Twinkles" },
  { snowTwinkles,           "Snow Twinkles" },
  { cloudTwinkles,          "Cloud Twinkles" },
  { incandescentTwinkles,   "Incandescent Twinkles" },

  // TwinkleFOX patterns
  { retroC9Twinkles,        "Retro C9 Twinkles" },
  { redWhiteTwinkles,       "Red & White Twinkles" },
  { blueWhiteTwinkles,      "Blue & White Twinkles" },
  { redGreenWhiteTwinkles,  "Red, Green & White Twinkles" },
  { fairyLightTwinkles,     "Fairy Light Twinkles" },
  { snow2Twinkles,          "Snow 2 Twinkles" },
  { hollyTwinkles,          "Holly Twinkles" },
  { iceTwinkles,            "Ice Twinkles" },
  { partyTwinkles,          "Party Twinkles" },
  { forestTwinkles,         "Forest Twinkles" },
  { lavaTwinkles,           "Lava Twinkles" },
  { fireTwinkles,           "Fire Twinkles" },
  { cloud2Twinkles,         "Cloud 2 Twinkles" },
  { oceanTwinkles,          "Ocean Twinkles" },

  { rainbow,                "Rainbow" },
  { rainbowWithGlitter,     "Rainbow With Glitter" },
  { rainbowSolid,           "Solid Rainbow" },
  { confetti,               "Confetti" },
  { sinelon,                "Sinelon" },
  { bpm,                    "Beat" },
  { juggle,                 "Juggle" },
  //{ fire,                   "Fire" },
  //{ water,                  "Water" },

  { showSolidColor,         "Solid Color" }
};

const uint8_t patternCount = ARRAY_SIZE(patterns);

#include "Fields.h"

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.setDebugOutput(true);

  //FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);     // for WS2812 (Neopixel)
  FastLED.addLeds<LED_TYPE, DATA_PIN, CLK_PIN, COLOR_ORDER>(leds, NUM_LEDS); // for APA102 (Dotstar)
  FastLED.setDither(true);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  EEPROM.begin(512);
  loadSettings();

  FastLED.setBrightness(brightness);

  //  irReceiver.enableIRIn(); // Start the receiver

  Serial.println();
  Serial.print( F("Heap: ") ); Serial.println(system_get_free_heap_size());
  Serial.print( F("Boot Vers: ") ); Serial.println(system_get_boot_version());
  Serial.print( F("CPU: ") ); Serial.println(system_get_cpu_freq());
  Serial.print( F("SDK: ") ); Serial.println(system_get_sdk_version());
  Serial.print( F("Chip ID: ") ); Serial.println(system_get_chip_id());
  Serial.print( F("Flash ID: ") ); Serial.println(spi_flash_get_id());
  Serial.print( F("Flash Size: ") ); Serial.println(ESP.getFlashChipRealSize());
  Serial.print( F("Vcc: ") ); Serial.println(ESP.getVcc());
  Serial.println();

  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), String(fileSize).c_str());
    }
    Serial.printf("\n");
  }

  // Set Hostname.
  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);
  WiFi.hostname(hostname);

  char hostnameChar[hostname.length() + 1];
  memset(hostnameChar, 0, hostname.length() + 1);

  for (uint8_t i = 0; i < hostname.length(); i++)
    hostnameChar[i] = hostname.charAt(i);

  //  MDNS.begin(hostnameChar);

  // Add service to MDNS-SD
  //  MDNS.addService("http", "tcp", 80);

  // Print hostname.
  Serial.println("Hostname: " + hostname);

  if (apMode)
  {
    WiFi.mode(WIFI_AP);

    // Do a little work to get a unique-ish name. Append the
    // last two bytes of the MAC (HEX'd) to "Thing-":
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    WiFi.softAPmacAddress(mac);
    String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                   String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
    macID.toUpperCase();
    String AP_NameString = "ESP8266-" + macID;

    char AP_NameChar[AP_NameString.length() + 1];
    memset(AP_NameChar, 0, AP_NameString.length() + 1);

    for (int i = 0; i < AP_NameString.length(); i++)
      AP_NameChar[i] = AP_NameString.charAt(i);

    WiFi.softAP(AP_NameChar, WiFiAPPSK);

    Serial.printf("Connect to Wi-Fi access point: %s\n", AP_NameChar);
    Serial.println("and open http://192.168.4.1 in your browser");
  }
  else
  {
    WiFi.mode(WIFI_STA);
    Serial.printf("Connecting to %s\n", ssid);
    WiFi.begin(ssid, pass);
    if (String(WiFi.SSID()) != String(ssid)) {
      WiFi.begin(ssid, pass);
    }
  }

  httpUpdateServer.setup(&webServer);

  webServer.on("/all", HTTP_GET, []() {
    String json = getFieldsJson(fields, fieldCount);
    webServer.send(200, "text/json", json);
  });

  webServer.on("/fieldValue", HTTP_GET, []() {
    String name = webServer.arg("name");
    String value = getFieldValue(name, fields, fieldCount);
    webServer.send(200, "text/json", value);
  });

  webServer.on("/fieldValue", HTTP_POST, []() {
    String name = webServer.arg("name");
    String value = webServer.arg("value");
    String newValue = setFieldValue(name, value, fields, fieldCount);
    webServer.send(200, "text/json", newValue);
  });

  webServer.on("/power", HTTP_POST, []() {
    String value = webServer.arg("value");
    setPower(value.toInt());
    sendInt(power);
  });

  webServer.on("/display_clock", HTTP_POST, []() {
    String value = webServer.arg("value");
    setDisplayClock(value.toInt());
    sendInt(display_clock);
  });

  webServer.on("/alarm", HTTP_POST, []() {
    String value = webServer.arg("value");
    setAlarm(value.toInt());
    sendInt(alarm);
  });

  webServer.on("/cooling", HTTP_POST, []() {
    String value = webServer.arg("value");
    cooling = value.toInt();
    broadcastInt("cooling", cooling);
    sendInt(cooling);
  });

  webServer.on("/sparking", HTTP_POST, []() {
    String value = webServer.arg("value");
    sparking = value.toInt();
    broadcastInt("sparking", sparking);
    sendInt(sparking);
  });

  webServer.on("/speed", HTTP_POST, []() {
    String value = webServer.arg("value");
    setSpeed(value.toInt());
    broadcastInt("speed", speed);
    sendInt(speed);
  });

  webServer.on("/twinkleSpeed", HTTP_POST, []() {
    String value = webServer.arg("value");
    twinkleSpeed = value.toInt();
    if (twinkleSpeed < 0) twinkleSpeed = 0;
    else if (twinkleSpeed > 8) twinkleSpeed = 8;
    broadcastInt("twinkleSpeed", twinkleSpeed);
    sendInt(twinkleSpeed);
  });

  webServer.on("/twinkleDensity", HTTP_POST, []() {
    String value = webServer.arg("value");
    twinkleDensity = value.toInt();
    if (twinkleDensity < 0) twinkleDensity = 0;
    else if (twinkleDensity > 8) twinkleDensity = 8;
    broadcastInt("twinkleDensity", twinkleDensity);
    sendInt(twinkleDensity);
  });

  webServer.on("/solidColor", HTTP_POST, []() {
    String r = webServer.arg("r");
    String g = webServer.arg("g");
    String b = webServer.arg("b");
    setSolidColor(r.toInt(), g.toInt(), b.toInt());
    sendString(String(solidColor.r) + "," + String(solidColor.g) + "," + String(solidColor.b));
  });

  webServer.on("/pattern", HTTP_POST, []() {
    String value = webServer.arg("value");
    setPattern(value.toInt());
    sendInt(currentPatternIndex);
  });

  webServer.on("/patternName", HTTP_POST, []() {
    String value = webServer.arg("value");
    setPatternName(value);
    sendInt(currentPatternIndex);
  });

  webServer.on("/palette", HTTP_POST, []() {
    String value = webServer.arg("value");
    setPalette(value.toInt());
    sendInt(currentPaletteIndex);
  });

  webServer.on("/paletteName", HTTP_POST, []() {
    String value = webServer.arg("value");
    setPaletteName(value);
    sendInt(currentPaletteIndex);
  });

  webServer.on("/brightness", HTTP_POST, []() {
    String value = webServer.arg("value");
    setBrightness(value.toInt());
    sendInt(brightness);
  });

  webServer.on("/timezone", HTTP_POST, []() {
    String value = webServer.arg("value");
    setTimeZone(value.toInt());
    sendInt(timezone);
  });

  webServer.on("/alarm_minute", HTTP_POST, []() {
    String value = webServer.arg("value");
    setAlarmMinute(value.toInt());
    sendInt(alarm_minute);
  });

  webServer.on("/alarm_hour", HTTP_POST, []() {
    String value = webServer.arg("value");
    setAlarmHour(value.toInt());
    sendInt(alarm_hour);
  });

  webServer.on("/autoplay", HTTP_POST, []() {
    String value = webServer.arg("value");
    setAutoplay(value.toInt());
    sendInt(autoplay);
  });

  webServer.on("/autoplayDuration", HTTP_POST, []() {
    String value = webServer.arg("value");
    setAutoplayDuration(value.toInt());
    sendInt(autoplayDuration);
  });

  //list directory
  webServer.on("/list", HTTP_GET, handleFileList);
  //load editor
  webServer.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/edit.htm")) webServer.send(404, "text/plain", "FileNotFound");
  });
  //create file
  webServer.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  webServer.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  webServer.on("/edit", HTTP_POST, []() {
    webServer.send(200, "text/plain", "");
  }, handleFileUpload);

  webServer.serveStatic("/", SPIFFS, "/", "max-age=86400");

  webServer.begin();
  Serial.println("HTTP web server started");

  webSocketsServer.begin();
  webSocketsServer.onEvent(webSocketEvent);
  Serial.println("Web socket server started");

  autoPlayTimeout = millis() + (autoplayDuration * 1000);


  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());  
}

void sendInt(uint8_t value)
{
  sendString(String(value));
}

void sendString(String value)
{
  webServer.send(200, "text/plain", value);
}

void broadcastInt(String name, uint8_t value)
{
  String json = "{\"name\":\"" + name + "\",\"value\":" + String(value) + "}";
  webSocketsServer.broadcastTXT(json);
}

void broadcastString(String name, String value)
{
  String json = "{\"name\":\"" + name + "\",\"value\":\"" + String(value) + "\"}";
  webSocketsServer.broadcastTXT(json);
}

void loop() {
  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy(random(65535));
  
  webSocketsServer.loop();
  webServer.handleClient();

  //  handleIrInput();

  current_time = last_update_time + (millis() - local_hack_ms)/ms_per_second;
  if(alarm){
    uint32_t tm = current_time + timezone * 3600;
    uint32_t time_of_day = tm % 86400;
    uint32_t alarm_time = alarm_hour * 3600 + alarm_minute * 60;
    //Serial.print(alarm_time);
    //Serial.print(":");
    //Serial.println(time_of_day);
    if(alarm_time == time_of_day){
      activate_alarm();
    }
    if(alarm_time + 5 * 60 == time_of_day){
      deactivate_alarm();
    }
  }
  if (power == 0) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    // FastLED.delay(15);
    return;
  }

  // EVERY_N_SECONDS(10) {
  //   Serial.print( F("Heap: ") ); Serial.println(system_get_free_heap_size());
  // }

  // change to a new cpt-city gradient palette
  EVERY_N_SECONDS( secondsPerPalette ) {
    gCurrentPaletteNumber = addmod8( gCurrentPaletteNumber, 1, gGradientPaletteCount);
    gTargetPalette = gGradientPalettes[ gCurrentPaletteNumber ];
  }

  EVERY_N_MILLISECONDS(40) {
    // slowly blend the current palette to the next
    nblendPaletteTowardPalette( gCurrentPalette, gTargetPalette, 8);
    gHue++;  // slowly cycle the "base color" through the rainbow
  }

  if (autoplay && (millis() > autoPlayTimeout)) {
    adjustPattern(true);
    autoPlayTimeout = millis() + (autoplayDuration * 1000);
  }

  // Call the current pattern function once, updating the 'leds' array
  patterns[currentPatternIndex].pattern();
  //patterns[0].pattern();
  if(display_clock){
    clock();
  }
  FastLED.show();

  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  if(clock_initialized == false ||
     (current_time - last_update_time > NTP_UPDATE_INTERVAL)){
    requestNTP();
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;

    case WStype_CONNECTED:
      {
        IPAddress ip = webSocketsServer.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        // webSocketsServer.sendTXT(num, "Connected");
      }
      break;

    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);

      // send message to client
      // webSocketsServer.sendTXT(num, "message here");

      // send data to all connected clients
      // webSocketsServer.broadcastTXT("message here");
      break;

    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\n", num, length);
      hexdump(payload, length);

      // send message to client
      // webSocketsServer.sendBIN(num, payload, lenght);
      break;
  }
}

void loadSettings()
{
  brightness = EEPROM.read(0);

  currentPatternIndex = EEPROM.read(1);
  if (currentPatternIndex < 0)
    currentPatternIndex = 0;
  else if (currentPatternIndex >= patternCount)
    currentPatternIndex = patternCount - 1;

  byte r = EEPROM.read(2);
  byte g = EEPROM.read(3);
  byte b = EEPROM.read(4);

  if (r == 0 && g == 0 && b == 0)
  {
  }
  else
  {
    solidColor = CRGB(r, g, b);
  }

  power = EEPROM.read(5);
  display_clock = EEPROM.read(12);
  alarm = EEPROM.read(11);

  autoplay = EEPROM.read(6);
  autoplayDuration = EEPROM.read(7);

  currentPaletteIndex = EEPROM.read(8);
  if (currentPaletteIndex < 0)
    currentPaletteIndex = 0;
  else if (currentPaletteIndex >= paletteCount)
    currentPaletteIndex = paletteCount - 1;

  speed = EEPROM.read(9);
  timezone = (int8_t)EEPROM.read(10);
  if(-12 >= timezone){
    timezone = 0;
  }
  if(12 <= timezone){
    timezone = 0;
  }
  alarm_hour = EEPROM.read(11);
  if(alarm_hour > 24){
    alarm_hour = 0;
  }
    
  alarm_minute = EEPROM.read(12);
  if(alarm_minute > 59){
    alarm_minute = 0;
  }
  //while(1)delay(100);
}

void setTimeZone(int8_t value){
  timezone = value;
  EEPROM.write(10, timezone);
  EEPROM.commit();
}

void setAlarmHour(int8_t value){
  alarm_hour = value;
  EEPROM.write(11, alarm_hour);
  EEPROM.commit();
}

void setAlarmMinute(int8_t value){
  alarm_minute = value;
  EEPROM.write(12, alarm_minute);
  EEPROM.commit();
}

void setPower(uint8_t value)
{
  power = value == 0 ? 0 : 1;

  EEPROM.write(5, power);
  EEPROM.commit();

  broadcastInt("power", power);
}

void setDisplayClock(uint8_t value)
{
  display_clock = value == 0 ? 0 : 1;

  EEPROM.write(12, display_clock);
  EEPROM.commit();

  broadcastInt("display_clock", display_clock);
}

uint8_t saved_pattern_index;
uint8_t saved_brightness;
bool alarm_active = false;
void activate_alarm(){
  if(!alarm_active){
    saved_pattern_index = currentPatternIndex;
    saved_brightness = brightness;
  }
  alarm_active = true;
  setPower(true);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  setDisplayClock(false);
  sunriseLevel = 0;
  setPatternName("Sunrise");
  setBrightness(100);
  Serial.println("Alarm!!");
}
bool deactivate_alarm(){
  if(alarm_active){
    alarm_active = false;
    setPattern(saved_pattern_index);
    //setPatternName("Solid Color");
    setDisplayClock(true);
    setBrightness(saved_brightness);
  }
}

void setAlarm(uint8_t value)
{
  bool orig = alarm;
  alarm = value == 0 ? 0 : 1;
  if(alarm == false && orig == true){
    deactivate_alarm();
  }
  EEPROM.write(11, alarm);
  EEPROM.commit();

  broadcastInt("alarm", alarm);
}

void setAutoplay(uint8_t value)
{
  autoplay = value == 0 ? 0 : 1;

  EEPROM.write(6, autoplay);
  EEPROM.commit();

  broadcastInt("autoplay", autoplay);
}

void setAutoplayDuration(uint8_t value)
{
  autoplayDuration = value;

  EEPROM.write(7, autoplayDuration);
  EEPROM.commit();

  autoPlayTimeout = millis() + (autoplayDuration * 1000);

  broadcastInt("autoplayDuration", autoplayDuration);
}

void setSolidColor(CRGB color)
{
  setSolidColor(color.r, color.g, color.b);
}

void setSolidColor(uint8_t r, uint8_t g, uint8_t b)
{
  solidColor = CRGB(r, g, b);

  EEPROM.write(2, r);
  EEPROM.write(3, g);
  EEPROM.write(4, b);
  EEPROM.commit();

  setPattern(patternCount - 1);

  broadcastString("color", String(solidColor.r) + "," + String(solidColor.g) + "," + String(solidColor.b));
}

// increase or decrease the current pattern number, and wrap around at the ends
void adjustPattern(bool up)
{
  if (up)
    currentPatternIndex++;
  else
    currentPatternIndex--;

  // wrap around at the ends
  if (currentPatternIndex < 0)
    currentPatternIndex = patternCount - 1;
  if (currentPatternIndex >= patternCount)
    currentPatternIndex = 0;

  if (autoplay == 0) {
    EEPROM.write(1, currentPatternIndex);
    EEPROM.commit();
  }

  broadcastInt("pattern", currentPatternIndex);
}

void setPattern(uint8_t value)
{
  if (value >= patternCount)
    value = patternCount - 1;

  currentPatternIndex = value;

  if (autoplay == 0) {
    EEPROM.write(1, currentPatternIndex);
    EEPROM.commit();
  }

  broadcastInt("pattern", currentPatternIndex);
}

bool setPatternName(String name)
{
  bool found = false;
  for (uint8_t i = 0; i < patternCount; i++) {
    Serial.print(i);
    Serial.print(" ");
    Serial.print(patterns[i].name);
    Serial.print(" ");
    Serial.println(patterns[i].name == name);
    if (patterns[i].name == name) {
      setPattern(i);
      found = true;
      break;
    }
  }
  if(!found){
    Serial.print("Pattern not found:");
    Serial.println(name);
  }
  return found;
}

void setPalette(uint8_t value)
{
  if (value >= paletteCount)
    value = paletteCount - 1;

  currentPaletteIndex = value;

  EEPROM.write(8, currentPaletteIndex);
  EEPROM.commit();

  broadcastInt("palette", currentPaletteIndex);
}

void setPaletteName(String name)
{
  for (uint8_t i = 0; i < paletteCount; i++) {
    if (paletteNames[i] == name) {
      setPalette(i);
      break;
    }
  }
}

void setSpeed(uint8_t value)
{
  speed = value;

  EEPROM.write(9, value);
  EEPROM.commit();

  broadcastInt("speed", speed);
}

void adjustBrightness(bool up)
{
  if (up && brightnessIndex < brightnessCount - 1)
    brightnessIndex++;
  else if (!up && brightnessIndex > 0)
    brightnessIndex--;

  brightness = brightnessMap[brightnessIndex];

  FastLED.setBrightness(brightness);

  EEPROM.write(0, brightness);
  EEPROM.commit();

  broadcastInt("brightness", brightness);
}

void setBrightness(uint8_t value)
{
  if (value > 255)
    value = 255;
  else if (value < 0) value = 0;

  brightness = value;

  FastLED.setBrightness(brightness);

  EEPROM.write(0, brightness);
  EEPROM.commit();

  broadcastInt("brightness", brightness);
}

void strandTest()
{
  static uint8_t i = 0;

  EVERY_N_SECONDS(1)
  {
    i++;
    if (i >= NUM_LEDS)
      i = 0;
  }

  fill_solid(leds, NUM_LEDS, CRGB::Black);

  leds[i] = solidColor;
}

void showSolidColor()
{
  fill_solid(leds, NUM_LEDS, solidColor);
}

// Patterns from FastLED example DemoReel100: https://github.com/FastLED/FastLED/blob/master/examples/DemoReel100/DemoReel100.ino

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 255 / NUM_LEDS);
}

void xyMatrixTest()
{
  FastLED.clear();

  static uint8_t x = 0;
  static uint8_t y = 0;

  leds[XY(x, y)] = CHSV(gHue, 255, 255);

  EVERY_N_MILLIS(30) {
    x++;
    if (x >= MatrixWidth) {
      x = 0;
      y++;
      if (y >= MatrixHeight) {
        y = 0;
      }
    }
  }
}

void verticalPalette() {
  uint8_t verticalHues = 256 / MatrixHeight;

  for (uint8_t y = 0; y < MatrixHeight; y++) {
    CRGB color = ColorFromPalette(palettes[currentPaletteIndex], beat8(speed) + (y * verticalHues));

    for (uint8_t x = 0; x < MatrixWidth; x++) {
      leds[XY(x, y)] = color;
    }
  }
}

void diagonalPalette() {
  uint8_t verticalHues = 256 / MatrixHeight;

  for (uint8_t y = 0; y < MatrixHeight; y++) {
    for (uint8_t x = 0; x < MatrixWidth; x++) {
      CRGB color = ColorFromPalette(palettes[currentPaletteIndex], beat8(speed) - ((x - y) * verticalHues));
      leds[XY(x, y)] = color;
    }
  }
}

void horizontalPalette() {
  uint8_t horizontalHues = 256 / MatrixWidth;

  for (uint8_t x = 0; x < MatrixWidth; x++) {
    CRGB color = ColorFromPalette(palettes[currentPaletteIndex], beat8(speed) - (x * horizontalHues));

    for (uint8_t y = 0; y < MatrixHeight; y++) {
      leds[XY(x, y)] = color;
    }
  }
}

void verticalGradientPalette() {
  uint8_t verticalHues = 256 / MatrixHeight;

  for (uint8_t y = 0; y < MatrixHeight; y++) {
    CRGB color = ColorFromPalette(gCurrentPalette, beat8(speed) + (y * verticalHues));

    for (uint8_t x = 0; x < MatrixWidth; x++) {
      leds[XY(x, y)] = color;
    }
  }
}

void diagonalGradientPalette() {
  uint8_t verticalHues = 256 / MatrixHeight;

  for (uint8_t y = 0; y < MatrixHeight; y++) {
    for (uint8_t x = 0; x < MatrixWidth; x++) {
      CRGB color = ColorFromPalette(gCurrentPalette, beat8(speed) - ((x - y) * verticalHues));
      leds[XY(x, y)] = color;
    }
  }
}

void horizontalGradientPalette() {
  uint8_t horizontalHues = 256 / MatrixWidth;

  for (uint8_t x = 0; x < MatrixWidth; x++) {
    CRGB color = ColorFromPalette(gCurrentPalette, beat8(speed) - (x * horizontalHues));

    for (uint8_t y = 0; y < MatrixHeight; y++) {
      leds[XY(x, y)] = color;
    }
  }
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void rainbowSolid()
{
  fill_solid(leds, NUM_LEDS, CHSV(gHue, 255, 255));
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  // leds[pos] += CHSV( gHue + random8(64), 200, 255);
  leds[pos] += ColorFromPalette(palettes[currentPaletteIndex], gHue + random8(64));
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(speed, 0, NUM_LEDS);
  static int prevpos = 0;
  CRGB color = ColorFromPalette(palettes[currentPaletteIndex], gHue, 255);
  if ( pos < prevpos ) {
    fill_solid( leds + pos, (prevpos - pos) + 1, color);
  } else {
    fill_solid( leds + prevpos, (pos - prevpos) + 1, color);
  }
  prevpos = pos;
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t beat = beatsin8( speed, 64, 255);
  CRGBPalette16 palette = palettes[currentPaletteIndex];
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle()
{
  static uint8_t    numdots =   4; // Number of dots in use.
  static uint8_t   faderate =   2; // How long should the trails be. Very low value = longer trails.
  static uint8_t     hueinc =  255 / numdots - 1; // Incremental change in hue between each dot.
  static uint8_t    thishue =   0; // Starting hue.
  static uint8_t     curhue =   0; // The current hue
  static uint8_t    thissat = 255; // Saturation of the colour.
  static uint8_t thisbright = 255; // How bright should the LED/display be.
  static uint8_t   basebeat =   5; // Higher = faster movement.

  static uint8_t lastSecond =  99;  // Static variable, means it's only defined once. This is our 'debounce' variable.
  uint8_t secondHand = (millis() / 1000) % 30; // IMPORTANT!!! Change '30' to a different value to change duration of the loop.

  if (lastSecond != secondHand) { // Debounce to make sure we're not repeating an assignment.
    lastSecond = secondHand;
    switch (secondHand) {
      case  0: numdots = 1; basebeat = 20; hueinc = 16; faderate = 2; thishue = 0; break; // You can change values here, one at a time , or altogether.
      case 10: numdots = 4; basebeat = 10; hueinc = 16; faderate = 8; thishue = 128; break;
      case 20: numdots = 8; basebeat =  3; hueinc =  0; faderate = 8; thishue = random8(); break; // Only gets called once, and not continuously for the next several seconds. Therefore, no rainbows.
      case 30: break;
    }
  }

  // Several colored dots, weaving in and out of sync with each other
  curhue = thishue; // Reset the hue values.
  fadeToBlackBy(leds, NUM_LEDS, faderate);
  for ( int i = 0; i < numdots; i++) {
    //beat16 is a FastLED 3.1 function
    leds[beatsin16(basebeat + i + numdots, 0, NUM_LEDS)] += CHSV(gHue + curhue, thissat, thisbright);
    curhue += hueinc;
  }
}

void fire()
{
  heatMap(HeatColors_p, true);
}

void water()
{
  heatMap(IceColors_p, false);
}

// Pride2015 by Mark Kriegsman: https://gist.github.com/kriegsman/964de772d64c502760e5
// This function draws rainbows with an ever-changing,
// widely-varying set of parameters.
void pride()
{
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for ( uint16_t i = 0 ; i < NUM_LEDS; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    CRGB newcolor = CHSV( hue8, sat8, bri8);

    uint16_t pixelnumber = i;
    pixelnumber = (NUM_LEDS - 1) - pixelnumber;

    nblend( leds[pixelnumber], newcolor, 64);
  }
}

void pride2()
{
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for (uint8_t x = 0; x < MatrixWidth; x++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    CRGB newcolor = CHSV( hue8, sat8, bri8);

    for (uint8_t y = 0; y < MatrixHeight; y++) {
      nblend(leds[XY(x, y)], newcolor, 64);
    }
  }
}

void radialPaletteShift()
{
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    // leds[i] = ColorFromPalette( gCurrentPalette, gHue + sin8(i*16), brightness);
    leds[i] = ColorFromPalette(gCurrentPalette, i + gHue, 255, LINEARBLEND);
  }
}

// based on FastLED example Fire2012WithPalette: https://github.com/FastLED/FastLED/blob/master/examples/Fire2012WithPalette/Fire2012WithPalette.ino
void heatMap(CRGBPalette16 palette, bool up)
{
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy(random(256));

  // Array of temperature readings at each simulation cell
  static byte heat[256];

  byte colorindex;

  // Step 1.  Cool down every cell a little
  for ( uint16_t i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((cooling * 10) / NUM_LEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for ( uint16_t k = NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if ( random8() < sparking ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160, 255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for ( uint16_t j = 0; j < NUM_LEDS; j++) {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
    colorindex = scale8(heat[j], 190);

    CRGB color = ColorFromPalette(palette, colorindex);

    if (up) {
      leds[j] = color;
    }
    else {
      leds[(NUM_LEDS - 1) - j] = color;
    }
  }
}

void addGlitter( uint8_t chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

///////////////////////////////////////////////////////////////////////

// Forward declarations of an array of cpt-city gradient palettes, and
// a count of how many there are.  The actual color palette definitions
// are at the bottom of this file.
extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];
extern const uint8_t gGradientPaletteCount;

uint8_t beatsaw8( accum88 beats_per_minute, uint8_t lowest = 0, uint8_t highest = 255,
                  uint32_t timebase = 0, uint8_t phase_offset = 0)
{
  uint8_t beat = beat8( beats_per_minute, timebase);
  uint8_t beatsaw = beat + phase_offset;
  uint8_t rangewidth = highest - lowest;
  uint8_t scaledbeat = scale8( beatsaw, rangewidth);
  uint8_t result = lowest + scaledbeat;
  return result;
}

void colorWaves()
{
  colorwaves( leds, NUM_LEDS, gCurrentPalette);
}

// ColorWavesWithPalettes by Mark Kriegsman: https://gist.github.com/kriegsman/8281905786e8b2632aeb
// This function draws color waves with an ever-changing,
// widely-varying set of parameters, using a color palette.
void colorwaves( CRGB* ledarray, uint16_t numleds, CRGBPalette16& palette)
{
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  // uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 300, 1500);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for ( uint16_t i = 0 ; i < numleds; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;
    uint16_t h16_128 = hue16 >> 7;
    if ( h16_128 & 0x100) {
      hue8 = 255 - (h16_128 >> 1);
    } else {
      hue8 = h16_128 >> 1;
    }

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    uint8_t index = hue8;
    //index = triwave8( index);
    index = scale8( index, 240);

    CRGB newcolor = ColorFromPalette( palette, index, bri8);

    uint16_t pixelnumber = i;
    pixelnumber = (numleds - 1) - pixelnumber;

    nblend( ledarray[pixelnumber], newcolor, 128);
  }
}

void colorWaves2()
{
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  // uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 300, 1500);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for (uint8_t x = 0; x < MatrixWidth; x++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;
    uint16_t h16_128 = hue16 >> 7;
    if ( h16_128 & 0x100) {
      hue8 = 255 - (h16_128 >> 1);
    } else {
      hue8 = h16_128 >> 1;
    }

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    uint8_t index = hue8;
    //index = triwave8( index);
    index = scale8( index, 240);

    CRGB newcolor = ColorFromPalette(gCurrentPalette, index, bri8);

    for (uint8_t y = 0; y < MatrixHeight; y++) {
      nblend(leds[XY(x, y)], newcolor, 128);
    }
  }
}

// Alternate rendering function just scrolls the current palette
// across the defined LED strip.
void palettetest( CRGB* ledarray, uint16_t numleds, const CRGBPalette16& gCurrentPalette)
{
  static uint8_t startindex = 0;
  startindex--;
  fill_palette( ledarray, numleds, startindex, (256 / NUM_LEDS) + 1, gCurrentPalette, 255, LINEARBLEND);
}


//################################################################### 
//  Clock Display
//################################################################### 

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
      }
      else{
      }
    }
  }
}

void colen(byte col){
  setPixelMask(2, col, true);
  setPixelMask(3, col, true);
  setPixelMask(5, col, true);
  setPixelMask(6, col, true);
  setPixelMask(2, col + 24, true);
  setPixelMask(3, col + 24, true);
  setPixelMask(5, col + 24, true);
  setPixelMask(6, col + 24, true);
}
void getHHMMSS(uint32_t tm, uint8_t *hhmmss){
  hhmmss[0] = (tm / 3600) % 24;
  hhmmss[1] = (tm / 60) % 60;
  hhmmss[2] = (tm) % 60;
}
void displayTime(uint32_t tm){
  uint8_t hhmmss[3];
  getHHMMSS(tm, hhmmss);
  if(hhmmss[0] > 9){
    digit( 1, hhmmss[0]/10);
  }
  digit( 6, hhmmss[0]%10);
  digit(13, hhmmss[1] / 10);
  digit(18, hhmmss[1] % 10);
  colen(11);
}

// set mask to all masked (b=false) or all unmasked (b = true)
void fillMask(bool b){
  for(int i = 0; i < NUM_LEDS; i++){
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
  }
}

void clock(){
  uint32_t tm = current_time + timezone * 3600;
  fillMask(false);
  displayTime(tm);
  apply_mask();
}
//################################################################### NTP Client
//  NTP Client
//################################################################### NTP Client


const char* ntpServerName = "time.nist.gov";
bool ntp_pending = false;
uint32_t ntp_request_sent_ms = 0;
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming and outgoing packets
const double LSB = 1./4294967296.;
const unsigned long seventyYears = 2208988800UL;
const uint32_t TOLLERANCE_MS = 100;
//const unsigned int localPort = 123;    // local port to listen for UDP packets

const uint32_t MEASURED_BIAS_ms = +400;
// uint32_t current_time;     // moved to top
// uint32_t last_update_time; // moved to top

void requestNTP(){
  //get a random server from the pool
  IPAddress nist_timeServerIP; // time.nist.gov NTP server address

  if(!ntp_pending){ // request
    WiFi.hostByName(ntpServerName, nist_timeServerIP); 
    Serial.print("nist_timeServerIP:");
    Serial.println(nist_timeServerIP);
    sendNTPpacket(nist_timeServerIP); // send an NTP packet to a time server
    Serial.println("NTP request sent");
    ntp_request_sent_ms = millis();
    ntp_pending = true;
  }
  if(millis() - ntp_request_sent_ms > 1){ // receive and parse
    ntp_pending = false;
    Serial.println("check for NTP data");
    int n_byte = udp.parsePacket();
    if(n_byte >= NTP_PACKET_SIZE) { // we have received ntp packet back!
      uint32_t receive_ms = millis();
      int32_t lag_ms = receive_ms - ntp_request_sent_ms;
      Serial.print("Lag (ms):");
      Serial.println(lag_ms);

      // We've received a packet, read the data from it
      udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

      //the timestamp starts at byte 40 of the received packet and is four bytes,
      // or two words, long. First, esxtract the two words:
      // inspect header
      for(int ii=0; ii<4; ii++){
	Serial.println(packetBuffer[ii], BIN);
      }
      
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = bytes2long(packetBuffer + 40);
      unsigned long hack_us = bytes2long(packetBuffer + 44) * 1e6 * LSB;
      unsigned long origin_stamp = bytes2long(packetBuffer + 24);
      unsigned long receipt_stamp = bytes2long(packetBuffer + 32);
      unsigned long transmit_stamp = bytes2long(packetBuffer + 40);

      // now convert NTP time into everyday time:
      // subtract seventy years:
      unsigned long epoch = secsSince1900 - seventyYears;
      uint32_t hack = epoch;
      double expect = local_hack + (local_hack_us / 1000. + receive_ms - local_hack_ms) / 1000.;
      //correction =  -lag_ms + last_lag_ms - dLag
      // expect += lag_ms/2000.;
      double got = hack + hack_us / 1e6;
      int diff_ms = (int)((got - expect) * 1000);                        /// say diff_ms = 300 ==> refernce is 300 ms ahead of local
      Serial.print("got - expect(ms): ");
      Serial.println((int)(diff_ms));
      last_update_time = hack;
      current_time = last_update_time;
      clock_initialized = true;
      
      uint32_t hack_ms = millis();

      local_hack = hack;
      local_hack_ms = hack_ms;
      local_hack_us = hack_us;
      last_lag_ms = lag_ms;
    }
    else{
      if(millis() - ntp_request_sent_ms > 1000){
	ntp_pending = false; // give up on this packet and requesta  new one
      }
      else{
	ntp_pending = true; // check back later
      }
    }
  }
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // 8 bytes of origin timestamp
  packetBuffer[24] = 1;
  packetBuffer[25] = 2;
  packetBuffer[26] = 3;
  packetBuffer[27] = 4;
  packetBuffer[28] = 5;
  packetBuffer[29] = 6;
  packetBuffer[30] = 7;
  packetBuffer[31] = 8;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

uint32_t bytes2long(byte *bytes){
  uint32_t out = 0;
  out = 0;
  for(int ii=0; ii<4; ii++){
    out |= (bytes[ii] << ((3 - ii) * 8));
  }
  return out;
}

void long2bytes(uint32_t l, byte *bytes){
  for(int ii=0; ii<4; ii++){
    bytes[ii] = (byte)(l >> (3 - ii) * 8);
  }  
}


