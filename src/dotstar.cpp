#include "dotstar.h"

#define BRIGHTNESS 90
#define DATAPIN    11
#define CLOCKPIN   13

DotStar::DotStar() {
  FastLED.addLeds<APA102, DATAPIN, CLOCKPIN, BGR, DATA_RATE_KHZ(800)>(
      leds[0], leds.Size()).setCorrection(0xF0F0FF);
  FastLED.setBrightness(BRIGHTNESS);
}

void DotStar::begin() {
  // ?
}

void DotStar::welcome() {
  uint8_t hue = 0;
  uint8_t val = 0;
  for (int16_t i = 0; i < 1024; i++) {
    hue = i % 255;
    val = (512 - abs(i - 512)) / 2;
    leds.DrawRectangle(6, 0, 11, 5, CHSV(hue,      255, val));
    leds.DrawRectangle(7, 1, 10, 4, CHSV(hue + 35, 255, val));
    leds.DrawRectangle(8, 2,  9, 3, CHSV(hue + 70, 255, val));
    FastLED.show();
  }
}
