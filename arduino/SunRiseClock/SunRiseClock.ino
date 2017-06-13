/*
 * USB animation server based on
 *   ESP8266 + FastLED + IR Remote: https://github.com/jasoncoon/esp8266-fastled-webserver
 *   Copyright (C) 2015-2016 Jason Coon
 * Ported by Justin Shaw
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <FastLED.h>
#include <Wire.h>
#include <RTClib.h>
FASTLED_USING_NAMESPACE

#include "GradientPalettes.h"

RTC_DS3231 rtc;
DateTime now;

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define CLOCK_MODE 1
#define SLEEP_MODE 2
#define ALARM_MODE 3
#define DAZZLE_MODE 4
uint8_t mode = CLOCK_MODE;


#define DATA_PIN      SCK
#define CLK_PIN       MOSI
#define LED_TYPE      APA102
#define COLOR_ORDER   BGR
#define MatrixWidth   24 * 2
#define MatrixHeight  8
#define NUM_LEDS      MatrixWidth * MatrixHeight

const bool MatrixSerpentineLayout = true;

#define MILLI_AMPS         2000     // IMPORTANT: set the max milli-Amps of your power supply (4A = 4000mA)
#define FRAMES_PER_SECOND  120 // here you can control the speed. With the Access Point / Web Server the animations run a bit slower.

CRGB leds[NUM_LEDS];
bool mask[NUM_LEDS];

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

// preceed with a call to fillMask(false);
// set mask to true where digit should light
void digit(byte start, byte d){
  byte row, col;
  for(col = 0; col < 4; col++){
    for(row = 0; row < 8; row++){
      if((digits4x8[d * 8 + row] >> col) & 1){
	togglePixelMask(row, col + start, true);
      }
      else{
      }
    }
  }
}

void displayNum(uint32_t n){
  digit( 0, (n / 10000) % 10);
  digit( 5, (n / 1000) % 10);
  digit(10, (n / 100) % 10);
  digit(15, (n / 10) % 10);
  digit(20, (n / 1) % 10);
}

void displayTime(uint32_t tm){
  uint8_t hh = (tm / 3600) % 12;
  uint8_t mm = (tm / 60) % 60;
  uint8_t ss = (tm) % 60;
  if(hh > 9){
    digit( 1, hh/10);
  }
  digit( 6, hh%10);
  setPixelMask(2, 11, true);
  setPixelMask(3, 11, true);
  setPixelMask(5, 11, true);
  setPixelMask(6, 11, true);
  digit(13, mm / 10);
  digit(18, mm % 10);

  if(hh > 9){
    digit( 1 + 24, hh/10);
  }
  digit( 6 + 24, hh%10);
  setPixelMask(2, 11 + 24, true);
  setPixelMask(3, 11 + 24, true);
  setPixelMask(5, 11 + 24, true);
  setPixelMask(6, 11 + 24, true);
  digit(13 + 24, mm / 10);
  digit(18 + 24, mm % 10);
}

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

bool autoplay = true;

uint8_t autoplayDuration = 10;
unsigned long autoPlayTimeout = 0;

uint8_t currentPaletteIndex = 0;

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

CRGB solidColor = CRGB::Blue;

uint16_t XY( uint8_t x, uint8_t y)
{
  y = 7 - y;
  x = 47 - x;
  uint16_t i;
  
  if ( MatrixSerpentineLayout == false) {
    i = (y * MatrixWidth) + x;
  }

  if ( MatrixSerpentineLayout == true) {
    if (y >= 8) {
      // x=0, y=8 ==> 56 * 8 + XY(55, 0)
      i = XY(111 - x,  15 - y);
    }
    else {
      //      if ( x & 0x01) {
      //	// odd rows run forwards
      //	i = (x * 8) + y;
      //      } else {
      //	// even columns run backwards
      //	uint8_t reverseY = (8 - 1) - y;
      //	i = (x * 8) + reverseY;
      //      }
      if ( x & 0x01) {
        // Odd columns run backwards
        uint8_t reverseY = (MatrixHeight - 1) - y;
        i = (x * MatrixHeight) + reverseY;
      } else {
        // Even rows run forwards
        i = (x * MatrixHeight) + y;
      }
    }
  }

  return i;
}

// scale the brightness of all pixels down
void dimAll(byte value)
{
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(value);
  }
}

typedef void (*Pattern)();
typedef Pattern PatternList[];
typedef struct {
  Pattern pattern;
  String name;
} PatternAndName;
typedef PatternAndName PatternAndNameList[];

#include "Twinkles.h"
#include "TwinkleFOX.h"
#include "Noise.h"

uint8_t brightness = 8;

// List of patterns to cycle through.  Each is defined as a separate function below.
PatternAndNameList clockPatterns = {
  { fireNoise, "Fire Noise" },
  { fireNoise2, "Fire Noise 2" },
  { lavaNoise, "Lava Noise" },
  { rainbowNoise, "Rainbow Noise" },
  { partyNoise, "Party Noise" },
  { cloudNoise, "Cloud Noise" },
  { showSolidColor,         "Solid Color" },
};

PatternAndNameList patterns = {
  { pride,                  "Pride" },
  { pride2,                 "Pride 2" },
  { colorWaves,             "Color Waves" },
  { colorWaves2,            "Color Waves 2" },

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
  { fire,                   "Fire" },
  { water,                  "Water" },
  { showSolidColor,         "Solid Color" }
};

const uint8_t patternCount = ARRAY_SIZE(patterns);
const uint8_t clockPatternCount = ARRAY_SIZE(clockPatterns);

uint8_t currentPatternIndex = 0; // Index number of which pattern is current

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


uint32_t alarm_tod = 5 * 3600 + 0 * 60;

void setup() {
  Serial.begin(115200);
  delay(100);
  Wire.begin();
  fillMask(true);
  FastLED.addLeds<LED_TYPE, DATA_PIN, CLK_PIN, COLOR_ORDER>(leds, NUM_LEDS); // for APA102 (Dotstar)
  FastLED.setDither(false);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  //rtc.adjust(DateTime(2017, 5, 21, 18, 41, 0));
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

// b = true -- unmask, b = false mask
void togglePixelMask(uint8_t row, uint8_t col, bool b){
  uint16_t pos = XY(col, (7 - row));
  mask[pos] = ! mask[pos];
}
void setPixelMask(uint8_t row, uint8_t col, bool b){
  if(row >= MatrixHeight){
  }
  else if(col >= MatrixWidth){
  }
  else{
    uint16_t pos = XY(col, (7 - row));
    if(pos < NUM_LEDS){
      mask[pos] = b;
    }
  }
}

void loop(){
  uint32_t tm, tod;
  
  tm = rtc.now().unixtime();
  tod = tm % 86400;
    
  autoplay = false;
  fillMask(false);
  if(mode == CLOCK_MODE){
    setPatternName("Cloud Noise");
    displayTime(tm);
  }
  if(mode == SLEEP_MODE){
    setPatternName("Fire Noise 2");    
    // displayTime(tm);
    //displayNum(alarm_tod - tod);
    if((alarm_tod < tod) && (tod < alarm_tod + 3600)){
      mode = ALARM_MODE;
    }
  }
  if(mode == ALARM_MODE){
    setPatternName("Incandescent Twinkles");
    //setPatternName("Retro C9 Twinkles");
    fillMask(true);
    if(density < 255){
      density++;
    }
    //displayTime(tm);
  }
  fastled_loop();
}

void fastled_loop() {

  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy(random(65535));
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
  apply_mask();

  // turn off dead pixels
  leds[64 * 6 - 1] = CRGB::Black;

  FastLED.show();

  // insert a delay to keep the framerate modest
  //FastLED.delay(1000 / FRAMES_PER_SECOND);
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
  Serial.print(millis());
  Serial.print(" ");
  Serial.println(patterns[currentPatternIndex].name);
  if (autoplay == 0) {
  }
}

void setPattern(uint8_t value)
{
  if (value >= patternCount)
    value = patternCount - 1;
  currentPatternIndex = value;
}

void setPatternName(String name)
{
  for(uint8_t i = 0; i < patternCount; i++) {
    if(patterns[i].name == name) {
      setPattern(i);
      break;
    }
  }
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
void heatMap(CRGBPalette16 palette, bool invert)
{
  EVERY_N_MILLIS(30) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);

    // Add entropy to random number generator; we use a lot of it.
    random16_add_entropy(random(256));

    // Array of temperature readings at each simulation cell
    static byte heat[MatrixWidth][MatrixHeight];

    byte colorindex;

    for (uint8_t x = 0; x < MatrixWidth; x++) {
      // Step 1.  Cool down every cell a little
      for ( uint8_t i = 0; i < MatrixHeight; i++) {
        heat[x][i] = qsub8( heat[x][i],  random8(0, ((cooling * 10) / MatrixHeight) + 2));
      }

      // Step 2.  Heat from each cell drifts 'up' and diffuses a little
      for ( uint8_t k = MatrixHeight - 1; k >= 2; k--) {
        heat[x][k] = (heat[x][k - 1] + heat[x][k - 2] + heat[x][k - 2] ) / 3;
      }

      // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
      if ( random8() < sparking ) {
        heat[x][1] = qadd8( heat[x][1], random8(160, 255) );
      }

      // Step 4.  Map from heat cells to LED colors
      for ( uint8_t j = 0; j < MatrixHeight; j++) {
        // Scale the heat value from 0-255 down to 0-240
        // for best results with color palettes.
        colorindex = scale8(heat[x][j], 190);

        CRGB color = ColorFromPalette(palette, colorindex);

        if (invert) {
          leds[XY(x, j)] = color;
        }
        else {
          leds[XY(x, (MatrixHeight - 1) - j)] = color;
        }
      }
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
