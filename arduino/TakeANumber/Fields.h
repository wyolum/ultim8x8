/*
   ESP8266 + FastLED + IR Remote: https://github.com/jasoncoon/esp8266-fastled-webserver
   Copyright (C) 2016 Jason Coon

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

uint8_t power = 1;
uint8_t display_clock = 1;
uint8_t alarm = 1;
uint8_t brightness = brightnessMap[brightnessIndex];
int8_t timezone = -5;
uint32_t take_a_number = 17;
uint8_t alarm_hour, alarm_minute;

//String setPower(String value) {
//  power = value.toInt();
//  if(power < 0) power = 0;
//  else if (power > 1) power = 1;
//  return String(power);
//}

String getPower() {
  return String(power);
}

String getDisplayClock() {
  return String(display_clock);
}

String getAlarm() {
  return String(alarm);
}

String getTakeANumber() {
  return String(take_a_number);
}

String getBrightness() {
  return String(brightness);
}

String getPattern() {
  return String(currentPatternIndex);
}

String getPatterns() {
  String json = "";

  for (uint8_t i = 0; i < patternCount; i++) {
    json += "\"" + patterns[i].name + "\"";
    if (i < patternCount - 1)
      json += ",";
  }

  return json;
}

String getPalette() {
  return String(currentPaletteIndex);
}

String getPalettes() {
  String json = "";

  for (uint8_t i = 0; i < paletteCount; i++) {
    json += "\"" + paletteNames[i] + "\"";
    if (i < paletteCount - 1)
      json += ",";
  }

  return json;
}

String getAutoplay() {
  return String(autoplay);
}

String getAutoplayDuration() {
  return String(autoplayDuration);
}

String getSolidColor() {
  return String(solidColor.r) + "," + String(solidColor.g) + "," + String(solidColor.b);
}

String getCooling() {
  return String(cooling);
}

String getSparking() {
  return String(sparking);
}

String getSpeed() {
  return String(speed);
}

String getTwinkleSpeed() {
  return String(twinkleSpeed);
}

String getTwinkleDensity() {
  return String(twinkleDensity);
}

FieldList fields = {
  { "power", "Power", BooleanFieldType, 0, 1, getPower },
  //{ "display_clock", "Clock", BooleanFieldType, 0, 1, getDisplayClock },
  { "take_a_number", "Now Serving:", NumberFieldType, 0, 99, getTakeANumber },
  {    "brightness",    "Brightness", NumberFieldType, 1, 255, getBrightness },
};

uint8_t fieldCount = ARRAY_SIZE(fields);
