#include "Snake.h"
#include <Wire.h>
#include <FastLED.h>
#include <I2CNavKey.h>

#include "MatrixMap.h"

#define DATA_PIN     MOSI
#define CLK_PIN      SCK
#define COLOR_ORDER BGR
#define LED_TYPE APA102
#define MILLI_AMPS 1000  // IMPORTANT: set the max milli-Amps 

void interact_loop();

// Constants
const int IntPin = 16; /* Definition of the interrupt pin*/

// Globals
bool mask[NUM_LEDS];

// navkey callbacks
//****************************************************************************
i2cNavKey navkey(0b0010000); /* Default address when no jumper are soldered */
void UP_Button_Pressed(i2cNavKey* p) {
  Serial.println("Button EAST Pressed!");
  snake_direction = EAST;
}

void DOWN_Button_Pressed(i2cNavKey* p) {
  Serial.println("Button WEST Pressed!");
  snake_direction = WEST;
}

void LEFT_Button_Pressed(i2cNavKey* p) {
  Serial.println("Button SOUTH Pressed!");
  snake_direction = SOUTH;
}

void RIGHT_Button_Pressed(i2cNavKey* p) {
  Serial.println("Button NORTH Pressed!");
  snake_direction = NORTH;
}

void CENTRAL_Button_Pressed(i2cNavKey* p) {
  Serial.println("Button Central Pressed!");
  if(GameOver == true){
    GameOver = false;
  }
}

void CENTRAL_Button_Double(i2cNavKey* p) {
  Serial.println("Button Central Double push!");
}

void Encoder_Rotate(i2cNavKey* p) {
  byte wheel_val = (byte)p->readCounterInt();
  Serial.println(wheel_val);
}


// end navkey callbacks
//*****************************************************************************

void splash(){
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  my_show();
  delay(500);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  my_show();
}
    
void my_show(){
  FastLED.show();
}

void navkey_setup(){
  pinMode(IntPin, INPUT);
  Wire.begin();
  Serial.begin(115200);
  Serial.println("**** Snake ****");
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

void led_setup(){
  FastLED.addLeds<LED_TYPE, DATA_PIN, CLK_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setDither(true);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
  FastLED.setBrightness(8);
  splash();
  for(int row = 0; row < MatrixHeight; row++){
    for(int col = 0; col < MatrixWidth; col++){
      leds[XY(col, row)] = CRGB::Blue;
      FastLED.show();
      delay(2);
    }
  }
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}

void snake_setup(){
  snacks.New();
}

Snake snake;

void setup(){
  led_setup();
  navkey_setup();
  snake_setup();
  snake.interact_callback = (*interact_loop);
}


void navkey_loop() {
  uint8_t enc_cnt;
  if (digitalRead(IntPin) == LOW) {
    navkey.updateStatus();
  }
}

void interact_loop(){
  navkey_loop();
}

void snake_loop(){
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  snake.move();
  snacks.draw();
  snake.draw();
  delay(30);
}
void loop(){
  interact_loop();
  snake_loop();
  
  my_show();
  delay(250);
}
