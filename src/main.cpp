#include "dotstar.h"
#include <Bounce2.h>
#include <EEPROM.h>
#include <PinChangeInterrupt.h>
#include <U8g2lib.h>

#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>

#define BOUNCE_WITH_PROMPT_DETECTION

#define BUTTON_R 0
#define BUTTON_Y 10
#define BUTTON_B 11
#define BUZZER 5

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R2);
DotStar dotstar;

Bounce2::Button button[] = {Bounce2::Button(), Bounce2::Button(),
                            Bounce2::Button()};

enum colors {
  blue,
  yellow,
  red,
  green
};
colors color = blue;

enum states {
  welcome,
  initialize,
  startLevel,
  initTrial,
  startWait,
  continueWait,
  startStim,
  continueStim,
  hit,
  falseAlarm,
  miss,
  correctRejection,
  endTrial,
  levelUp,
  gameOver,
  timeToSleep,
  goToSleep
};
states state = welcome;

#define LVL_ONE_MILLIS 1000
#define START_LIVES 5
#define TRIALS_PER_LEVEL 10

volatile uint32_t lastInteraction;
uint32_t thisMillis, waitMillis, startStimMillis, startWaitMillis, stimMillis;
uint16_t score, highscore;
int8_t answer, trial;
uint8_t level, lives;
bool doBuzz, allCorrect = false;
char buf[20];

void wake() {
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(BUTTON_R));
  detachPCINT(digitalPinToPCINT(BUTTON_B));
  detachPCINT(digitalPinToPCINT(BUTTON_Y));
  lastInteraction = millis();
  state = welcome;
}

void jiggle(const char *text) {
  size_t len = strlen(text);
  if (len < 0) {
    return;
  }
  uint16_t x = (128 - u8g2.getStrWidth(text)) / 2;
  static const uint16_t y = 23;
  static bool yStart = false;
  bool yShift = yStart;

  u8g2.clearBuffer();
  u8g2.setCursor(x, y);
  for (uint8_t i = 0; i < len; i++) {
    u8g2.setCursor(u8g2.tx, (yShift) ? y + 1 : y);
    u8g2.print(text[i]);
    yShift = !yShift;
  }
  u8g2.sendBuffer();

  yStart = !yStart;
}

void jiggle(const char *text, uint32_t duration) {
  const uint32_t period = 80;
  int32_t leftOver = duration;
  while (leftOver > period) {
    jiggle(text);
    leftOver -= period;
    delay(period);
  }
  if (leftOver > 0) {
    jiggle(text);
    delay(leftOver);
  }
  u8g2.clearDisplay();
}

void setup() {
  Serial.begin(9600);

  pinMode(BUZZER, OUTPUT);
  button[0].attach(BUTTON_B, INPUT_PULLUP);
  button[1].attach(BUTTON_Y, INPUT_PULLUP);
  button[2].attach(BUTTON_R, INPUT_PULLUP);
  for (auto b : button) {
    b.interval(5);
    b.setPressedState(LOW);
  }

  u8g2.begin();
  u8g2.setFont(u8g2_font_michaelmouse_tu);
  u8g2.clearDisplay();

  randomSeed(analogRead(A0));

  dotstar.begin();

  EEPROM.get(0, highscore);
}

void loop() {
  thisMillis = millis();

  // update buttons
  for (uint8_t i = 0; i < 3; i++) {
    button[i].update();
    if (button[i].pressed()) {
      answer = i;
      break;
    }
    answer = -1;
  }

  // should we go to sleep?
  // if (thisMillis-lastInteraction > 60000) {
  //   state = STATE_GO_TO_SLEEP;
  // }

  switch (state) {
  case welcome:
    u8g2.setPowerSave(0);
    // dotstar.welcome();
    jiggle("MR. CHICKEN!", 1000);
    delay(500);

  case initialize:
    lastInteraction = thisMillis;
    level = 1;
    stimMillis = LVL_ONE_MILLIS;

  case startLevel:
    lives = START_LIVES;
    if (allCorrect) {
      jiggle("BONUS LIFE!", 1000);
      lives++;
    }
    sprintf(buf, "LEVEL %02d", level);
    jiggle(buf, 1000);
    delay(500);
    jiggle("READY", stimMillis);
    jiggle("SET", stimMillis);
    jiggle("GO!", stimMillis);
    state = initTrial;
    trial = 0;
    allCorrect = true;

  case initTrial:
    trial++;
    answer = -1;
    if (random(0, 20) < 1) {
      color = green;
      doBuzz = true;
    } else {
      color = (colors) random(0, 3);
      doBuzz = random(0, 2) > 0;
    }
    state = startWait;

  case startWait:
    startWaitMillis = thisMillis;
    waitMillis = random(1000, 3000);
    state = continueWait;

  case continueWait:
    if (answer > -1) {
      startWaitMillis = thisMillis;
    } else if (thisMillis - startWaitMillis > waitMillis) {
      state = startStim;
    }
    break;

  case startStim:
    startStimMillis = thisMillis;
    dotstar.stim(color);
    if (doBuzz) {
      tone(BUZZER, 3000, 25);
    }
    state = continueStim;

  case continueStim:
    if (answer > -1) {
      if (doBuzz && answer == color) {
        state = hit;
      } else {
        state = falseAlarm;
      }
      dotstar.clear();
      noTone(BUZZER);
    } else if (thisMillis - startStimMillis > stimMillis) {
      if (doBuzz && color != green) {
        state = miss;
      } else {
        state = correctRejection;
      }
      dotstar.clear();
      noTone(BUZZER);
    }
    break;

  case hit:
    jiggle("HIT!", 400);
    score++;
    state = endTrial;
    break;

  case falseAlarm:
    jiggle("FALSE ALARM", 400);
    lives--;
    trial--;
    allCorrect = false;
    state = endTrial;
    break;

  case miss:
    jiggle("MISS!", 400);
    lives--;
    trial--;
    allCorrect = false;
    state = endTrial;
    break;

  case correctRejection:
    jiggle("CORRECT", 400);
    score++;
    state = endTrial;
    break;

  case endTrial:
    if (lives == 0) {
      state = gameOver;
    } else if (trial == TRIALS_PER_LEVEL && level < 99) {
      state = levelUp;
    } else {
      state = initTrial;
    }
    break;

  case levelUp:
    stimMillis = stimMillis * 9 / 10;
    state = startLevel;
    level++;
    break;

  case gameOver:
    jiggle("GAME OVER", 1000);
    sprintf(buf, "SCORE: %d", score);
    jiggle(buf, 1000);
    if (score > highscore) {
      jiggle("NEW RECORD!", 1000);
      highscore = score;
      EEPROM.put(0, score);
    }
    state = goToSleep;
    break;

  // ----------------------
    break;
  case timeToSleep:
    jiggle("ZZZ", 500);
    state = goToSleep;
    // no break!

  case goToSleep:
    // adapted from https://www.gammon.com.au/power

    // clear OLED & dotstars
    u8g2.clearDisplay();
    u8g2.setPowerSave(1);
    dotstar.clear();

    ADCSRA = 0; // disable ADC

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();

    // Do not interrupt before we go to sleep, or the
    // ISR will detach interrupts and we won't wake.
    noInterrupts();

    // clear any outstanding interrupts
    EIFR |= bit(INTF0);
    PCIFR |= bit(digitalPinToPCICRbit(BUTTON_B));
    PCIFR |= bit(digitalPinToPCICRbit(BUTTON_Y));

    // attach interrupts
    attachInterrupt(digitalPinToInterrupt(BUTTON_R), wake, LOW);
    attachPCINT(digitalPinToPCINT(BUTTON_B), wake, CHANGE);
    attachPCINT(digitalPinToPCINT(BUTTON_Y), wake, CHANGE);

    interrupts(); // one cycle
    sleep_cpu();  // one cycle, CPU is ASLEEP and waiting for interrupts

  default:
    break;
  }
}
