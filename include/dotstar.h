#include <FastLED.h>
#include <LEDMatrix.h>

struct DotStar {
public:
  DotStar(void);
  void begin(void);
  void welcome(void);
  void square(const CRGB &, bool);
  void stim(uint8_t type);
  void clear();

private:
  cLEDMatrix<12, 6, HORIZONTAL_MATRIX> leds;
};
