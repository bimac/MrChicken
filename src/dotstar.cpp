#include "dotstar.h"

DotStar::DotStar() {
  FastLED.addLeds<APA102, BGR>(leds[0], leds.Size());
}

void DotStar::begin() {
  pinMode(17, OUTPUT);
  FastLED.clear(true);
  FastLED.setBrightness(80);
}

void DotStar::welcome() {
  uint8_t hue = 0;
  uint8_t val = 0;
  for (int16_t i = 0; i < 255; i++) {
    hue = i % 255;
    val = (512 - abs(i - 255)) / 2;
    Serial.println(val);
    leds.DrawRectangle(6, 0, 11, 5, CHSV(hue,      255, val));
    leds.DrawRectangle(7, 1, 10, 4, CHSV(hue + 35, 255, val));
    leds.DrawRectangle(8, 2,  9, 3, CHSV(hue + 70, 255, val));
    FastLED.show();
  }
  FastLED.clear(true);
}

void DotStar::square(const CRGB & color, bool show) {
  leds.DrawFilledRectangle(6, 0, 11, 5, color);
  if (show) {
    FastLED.show();
  }
}

void DotStar::stim(uint8_t type) {
  switch (type) {
    case 0: square(CRGB::Blue,   true); break;
    case 1: square(CRGB::Yellow, true); break;
    case 2: square(CRGB::Red,    true); break;
    case 3: square(CRGB::Green,  true); break;
  }
}

void DotStar::clear() {
  FastLED.clear(true);
}
