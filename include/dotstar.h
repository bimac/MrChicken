#include <FastLED.h>
#include <LEDMatrix.h>

struct DotStar {
public:
  DotStar(void);
  void begin(void);
  void welcome(void);

private:
  cLEDMatrix<12, 6, HORIZONTAL_MATRIX> leds;
};
