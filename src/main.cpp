#include "dotstar.h"
#include <Arduino.h>
#include <Bounce2.h>
#include <SPI.h>
#include <U8g2lib.h>

#define BOUNCE_WITH_PROMPT_DETECTION

#define BUTTON_R 9
#define BUTTON_Y 6
#define BUTTON_B 5

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R2);
DotStar dotstar;

Bounce2::Button red = Bounce2::Button();
Bounce2::Button yellow = Bounce2::Button();
Bounce2::Button blue = Bounce2::Button();

void setup() {
  Serial.begin(9600);

  red.attach(BUTTON_R, INPUT_PULLUP);
  red.interval(5);
  red.setPressedState(LOW);

  yellow.attach(BUTTON_Y, INPUT_PULLUP);
  yellow.interval(5);
  yellow.setPressedState(LOW);

  blue.attach(BUTTON_B, INPUT_PULLUP);
  blue.interval(5);
  blue.setPressedState(LOW);

  u8g2.begin();
  u8g2.setFont(u8g2_font_logisoso30_tr);
  u8g2.drawStr(0, 32, "WELCOME");
  u8g2.sendBuffer();

  dotstar.welcome();
  u8g2.clearDisplay();
}

void loop() {
  red.update();
  yellow.update();
  blue.update();

  if (red.pressed()) {
    Serial.println("RED");
  }
  if (yellow.pressed()) {
    Serial.println("YELLOW");
  }
  if (blue.pressed()) {
    Serial.println("BLUE");
  }

}
