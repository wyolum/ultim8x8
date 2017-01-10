// Adafruit IO Subscription Example


/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"
#include <Adafruit_DotStar.h>
#include <SPI.h>
#define PIXEL_COUNT 8*8*9// Number of LEDs in strip


// Here's how to control the LEDs from any two pins:
#define DATAPIN    16
#define CLOCKPIN   15
Adafruit_DotStar pixels = Adafruit_DotStar(PIXEL_COUNT, DOTSTAR_BGR);

// set up the 'feed
AdafruitIO_Feed *color = io.feed("PanelColor");
AdafruitIO_Feed *brightness = io.feed("brightness");

void setup() {

  // start the serial connection
  Serial.begin(115200);

  pixels.setBrightness(10);
  pixels.begin(); // Initialize pins for output
  pixels.show();  // Turn all LEDs off ASAP
  

  Serial.print("Connecting to Adafruit IO");

  // connect to io.adafruit.com
  io.connect();

  // set up a message handler for the count feed.
  // the handleMessage function (defined below)
  // will be called whenever a message is
  // received from adafruit io.
  color->onMessage(handleColor);
  brightness->onMessage(handleBrightness);

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());

}

// other global state. largely cribbed from adafruit's toilet bowl light example
int red = 0;                 // RGB color for the current animation.
int green = 0;
int blue = 0;
int animation = 0;           // Current animation (0 = none, 1 = pulse, 2 = rainbow cycle).
int pulsePeriodMS = 0;       // Period of time (in MS) for the pulse animation.

// Function to set all the apa102 to the specified color.
void lightPixels(uint32_t color) {
  Serial.print("setting color: ");
  Serial.println(color,HEX);
  for (int i=0; i<PIXEL_COUNT; ++i) {
    pixels.setPixelColor(i, color);
  }
  Serial.println("calling show");
  pixels.show();
  pixels.show();
}

// Function to parse a hex byte value from a string.
// The passed in string MUST be at least 2 characters long!
// If the value can't be parsed then -1 is returned, otherwise the
// byte value is returned.
int parseHexByte(char* data) {
  char high = tolower(data[0]);
  char low = tolower(data[1]);
  uint8_t result = 0;
  // Parse the high nibble.
  if ((high >= '0') && (high <= '9')) {
    result += 16*(high-'0');
  }
  else if ((high >= 'a') && (high <= 'f')) {
    result += 16*(10+(high-'a'));
  }
  else {
    // Couldn't parse the high nibble.
    return -1;
  }
  // Parse the low nibble.
  if ((low >= '0') && (low <= '9')) {
    result += low-'0';
  }
  else if ((low >= 'a') && (low <= 'f')) {
    result += 10+(low-'a');
  }
  else {
    // Couldn't parse the low nibble.
    return -1;
  }
  return result;
}

// Linear interpolation of value y within range y0...y1 given a value x
// and the range x0...x1.
float lerp(float x, float y0, float y1, float x0, float x1) {
  return y0 + (y1-y0)*((x-x0)/(x1-x0));
}

// Pulse the pixels from their color down to black (off) and back
// up every pulse period milliseconds.
void pulseAnimation() {
  // Calculate how far we are into the current pulse period.
  int n = millis() % pulsePeriodMS;
  // Pulse up or down depending on how far along into the period.
  if (n < (pulsePeriodMS/2)) {
    // In the first half so pulse up.
    // Interpolate between black/off and full color using n.
    uint8_t cr = (uint8_t)lerp(n, 0, red,   0, pulsePeriodMS/2-1);
    uint8_t cg = (uint8_t)lerp(n, 0, green, 0, pulsePeriodMS/2-1);
    uint8_t cb = (uint8_t)lerp(n, 0, blue,  0, pulsePeriodMS/2-1);
    // Light the pixels.
    lightPixels(pixels.Color(cr, cg, cb));
  }
  else {
    // In the second half so pulse down.
    // Interpolate between full color and black/off color using n.
    uint8_t cr = (uint8_t)lerp(n, red,   0, pulsePeriodMS/2, pulsePeriodMS-1);
    uint8_t cg = (uint8_t)lerp(n, green, 0, pulsePeriodMS/2, pulsePeriodMS-1);
    uint8_t cb = (uint8_t)lerp(n, blue,  0, pulsePeriodMS/2, pulsePeriodMS-1);
    // Light the pixels.
    lightPixels(pixels.Color(cr, cg, cb));
  }
  
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void rainbowAnimation() {
  // Assume the rainbow cycles every 2.56 seconds so there's a
  // 10 millisecond delay every color change.
  int n = (millis()/10) % 256;
//  lightPixels(Wheel(n));
  for (int i =0; i < pixels.numPixels(); i++){
    pixels.setPixelColor(i,Wheel((i+n)&255));
  }
  pixels.show();
  pixels.show();
}
void loop() {
  // Do any NeoPixel animation logic.
  if (animation == 1) {
    pulseAnimation();
  }
  else if (animation == 2) {
    rainbowAnimation();
  }
  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();

}
void handleBrightness(AdafruitIO_Data *message) {

  char *data = (char*)message->value();
  Serial.print("received <- ");
  Serial.println(data);

  int dataLen = strlen(data);
  Serial.print("Got: ");
  Serial.println(data);
  if (dataLen < 1) {
  // Stop processing if not enough data was received.
     return;
  }
  String dataStr = String(data);
  pixels.setBrightness(dataStr.toInt());
  pixels.show();
  pixels.show();
  
}
// this function is called whenever a 'PanelColor' message
// is received from Adafruit IO. it was attached to
// the counter feed in the setup() function above.
void handleColor(AdafruitIO_Data *message) {

  char *data = (char*)message->value();
  Serial.print("received <- ");
  Serial.println(data);

      int dataLen = strlen(data);
      Serial.print("Got: ");
      Serial.println(data);
      if (dataLen < 1) {
        // Stop processing if not enough data was received.
        return;
      }
      // Check the first character to determine the light change command.
      switch (data[0]) {
        case '#':
          // Solid color.
          // Expect 6 more characters with the hex red, green, blue color.
          if (dataLen >= 7) {
            // Parse out the RGB color bytes.
            int r = parseHexByte(&data[1]);
            int g = parseHexByte(&data[3]);
            int b = parseHexByte(&data[5]);
            if ((r < 0) || (g < 0) || (b < 0)) {
              // Couldn't parse the color, stop processing.
              break; 
            }
            // Light the pixels!
            lightPixels(pixels.Color(r, g, b));
            // Change the animation to none/stop animating.
            animation = 0;
          }
          break;
        case 'P':
          // Pulse animation.
          // Expect 8 more characters with the hex red, green, blue color, and
          // a byte value with the frequency to pulse within a 10 second period.
          // I.e. to make it the light pulse once every 2 seconds send the value
          // 5 so that the light pulses 5 times within a ten second period (every
          // 2 seconds).
          if (dataLen >= 9) {
            // Parse out the RGB color and frequency bytes.
            int r = parseHexByte(&data[1]);
            int g = parseHexByte(&data[3]);
            int b = parseHexByte(&data[5]);
            int f = parseHexByte(&data[7]);
            if ((r < 0) || (g < 0) || (b < 0) || (f < 0)) {
              // Couldn't parse the data, stop processing.
              break; 
            }
            // Change the color for the pulse animation.
            red   = r;
            green = g;
            blue  = b;
            // Calculate the pulse length in milliseconds from the specified frequency.
            pulsePeriodMS = (10.0 / (float)f) * 1000.0;
            // Change the animation to pulse.
            animation = 1;
          }
          break;
        case 'R':
          // Rainbow cycle animation.
          animation = 2;
          break;
      }
}
