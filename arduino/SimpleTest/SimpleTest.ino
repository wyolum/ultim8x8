#include "FastLED.h"

FASTLED_USING_NAMESPACE

// minimal FastLED program doing simple pixel setting.

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    MOSI
#define CLK_PIN  SCK
#define LED_TYPE    APA102
#define COLOR_ORDER BGR

#define NUM_LEDS    (3*64)
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          10

void setup() {
  //delay(1000); // 3 second delay for recovery
  Serial.begin(115200);
  
  FastLED.addLeds<APA102,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}

  
void loop()
{
  Serial.println("Setting Red");
  for (int i =0; i < NUM_LEDS; i ++){
    leds[i] = CRGB(255,0,0);
     FastLED.show();  
     delay(20);
  }
  Serial.println("Setting Green");
    for (int i =0; i < NUM_LEDS; i ++){
    leds[i] = CRGB(0,255,0);
     FastLED.show();  
     delay(20);
  }
  Serial.println("Setting Blue");
    for (int i =0; i < NUM_LEDS; i ++){
    leds[i] = CRGB(0,0,255);
     FastLED.show();  
     delay(20);
  }

}

