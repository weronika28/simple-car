#include <Screen.h>
#include <App.h>
#include <TFT_eSPI.h>
#include <RCCar.h>
#include <Arduino.h>

#define TOUCH_IRQ 35  // T_IRQ

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_SIZE 2

#define BG_COLOR 0x000022
#define BORDER_COLOR 0x0000dd

TFT_eSPI tft = TFT_eSPI();

int displayedMaxSpeed = 160;
int maxSpeedAnimationTimer = 0;

int displayedLeft = -1, displayedRight = -1;

int temperatureTimeUpdateTimer = 0;

volatile bool touchFlag = false;
void IRAM_ATTR touchIsr() {
  touchFlag = true;
}

Screen::Screen(App* appParam) {
  this->app = appParam;
}

void Screen::setup() {
  pinMode(TOUCH_IRQ, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TOUCH_IRQ), touchIsr, FALLING);

  tft.init();
  tft.setRotation(3);

  tft.fillScreen(BG_COLOR);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);

  tft.drawFastHLine(0, 30, 320, BORDER_COLOR);

  setConnectionStatus(false);
}

void Screen::loop() {
  touchLoop();

  if (app->currentScreen == SCREEN_CAR) {
    updateSpeedBar();
  }

  if (app->currentScreen == SCREEN_DHT) {
    if (temperatureTimeUpdateTimer < millis()) {
      temperatureTimeUpdateTimer = millis() + 1000;
      updateTemperatureUpdateTime();
    }
  }
}

void Screen::onScreenChange() {
  tft.fillRect(0, 31, 320, 209, BG_COLOR);

  if (app->currentScreen == SCREEN_CAR) {
    displayedLeft == -1;
    displayedRight == -1;
    this->updateSpeed();
    this->displaySpeedBar();
    this->displayCar();
  }

  if (app->currentScreen == SCREEN_DHT) {
    this->updateTemperatureScreen();
  }

  displayMenu();
}

void Screen::updateSpeedBar() {
  if (displayedMaxSpeed == app->currentMaxSpeed || maxSpeedAnimationTimer > millis())
    return;

  maxSpeedAnimationTimer = millis() + 50;

  displayedMaxSpeed = calculateNextAnimationFrame(displayedMaxSpeed, app->currentMaxSpeed, 7, 5);

  displaySpeedBar();
}

void Screen::updateSpeed() {
  if (app->currentScreen != SCREEN_CAR)
    return;

  if (displayedLeft == app->left && displayedRight == app->right)
    return;

  tft.setTextSize(2);
  tft.setTextDatum(CC_DATUM);

  if (displayedLeft != app->left) {
    tft.fillRect(80, 125, 60, 20, BG_COLOR);
    tft.drawString(String(app->left), 110, 135, 1);
    displayedLeft = app->left;
  }

  if (displayedRight != app->right) {
    tft.fillRect(180, 125, 60, 20, BG_COLOR);
    tft.drawString(String(app->right), 210, 135, 1);
    displayedRight = app->right;
  }

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);
}

int Screen::calculateNextAnimationFrame(int current, int target, int divider, int min) {
  int step = (target - current) / divider;

  if (step < min && step > 0) {
    step = min;
  }
  if (step > -min && step < 0) {
    step = -min;
  }

  if (abs(step) > abs(target - current) || step == 0) {
    return target;
  }

  return current + step;
}

void Screen::touchLoop() {
  if (!touchFlag) return;

  touchFlag = false;
  static uint32_t lastTouchCheck = 0;
  static bool wasTouched = false;

  if (millis() - lastTouchCheck < 50) {
    return;
  }

  lastTouchCheck = millis();

  uint16_t touchX, touchY;
  if (tft.getTouch(&touchX, &touchY)) {
    if (!wasTouched)
      handleTouch((int)(touchX - 10), (int)(SCREEN_HEIGHT - touchY - 10));
    wasTouched = true;
  } else {
    wasTouched = false;
  }
}

void Screen::handleTouch(int x, int y) {
  Serial.printf("touched x:%d y:%d\n", x, y);
  if (x < 50 && y > 30 && app->currentScreen == SCREEN_CAR)
    onSpeedBarClick(y);

  if (x > 265 && y > 30)
    onNavClick(y);
}

void Screen::onSpeedBarClick(int y) {
  y = max(40, min(210, y)) - 40;

  int progress = 190 - y;

  int speed = map(progress, 0, 190, MIN_SPEED, MAX_SPEED);

  app->setCurrentMaxSpeed(speed);
}

void Screen::displaySpeedBar() {
  int progress = map(displayedMaxSpeed, MIN_SPEED, MAX_SPEED, 0, 190);

  tft.fillRoundRect(10, 40, 30, 190, 10, BG_COLOR);
  tft.drawRoundRect(10, 40, 30, 190, 10, BORDER_COLOR);
  tft.fillRoundRect(10, 40 + 190 - progress, 30, progress, 10, BORDER_COLOR);
}

void Screen::setConnectionStatus(bool connected) {
  if (this->connected == connected)
    return;

  this->connected = connected;

  tft.fillRect(220, 4, 100, 22, BG_COLOR);

  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(connected ? TFT_GREEN : TFT_RED);
  tft.drawString(connected ? "Polaczony" : "Niepolaczony", 310, 15, 2);
  tft.setTextColor(0xffffff);
  tft.setTextDatum(TL_DATUM);
}

void Screen::displayCar() {
  // 120x150
  // middle line
  tft.fillRect(155, 75, 10, 115, BORDER_COLOR);
  tft.fillRoundRect(150, 115, 20, 40, 5, BORDER_COLOR);

  // top line
  tft.fillRoundRect(120, 75, 80, 10, 5, BORDER_COLOR);

  // bottom line
  tft.fillRoundRect(120, 185, 80, 10, 5, BORDER_COLOR);

  // wheels
  tft.fillRoundRect(100, 60, 20, 40, 8, 0x0000aa);
  tft.fillRoundRect(200, 60, 20, 40, 8, 0x0000aa);

  tft.fillRoundRect(100, 170, 20, 40, 8, 0x0000aa);
  tft.fillRoundRect(200, 170, 20, 40, 8, 0x0000aa);
}

void Screen::updateTemperatureScreen() {
  if (this->app->currentScreen != SCREEN_DHT)
    return;

  tft.setTextSize(2);

  tft.fillRect(20, 80, 250, 40, BG_COLOR);
  tft.setCursor(20, 80);

  if (app->temperature > -1000) {
    tft.printf("Temperatura: %.2f C", app->temperature);
  } else {
    tft.printf("Temperatura: -");
  }

  tft.fillRect(20, 120, 250, 40, BG_COLOR);
  tft.setCursor(20, 120);

  if (app->humidity > -1000) {
    tft.printf("Wilgotnosc: %.2f%%", app->humidity);
  } else {
    tft.printf("Wilgotnosc: -");
  }

  tft.setTextSize(1);
  this->updateTemperatureUpdateTime();
}

void Screen::updateTemperatureUpdateTime() {
  if (this->app->currentScreen != SCREEN_DHT)
    return;
  if (this->app->lastDhtUpdate < 0)
    return;

  tft.fillRect(20, 200, 240, 20, BG_COLOR);
  tft.setCursor(20, 200);

  int seconds = (millis() - this->app->lastDhtUpdate) / 1000;
  int minutes = seconds / 60;
  seconds %= 60;

  tft.setTextColor(BORDER_COLOR);
  if (minutes > 0) {
    tft.printf("Ostatnia aktualizacja: %dm %ds temu", minutes, seconds);
  } else {
    tft.printf("Ostatnia aktualizacja: %ds temu", seconds);
  }
  tft.setTextColor(TFT_WHITE);
}

void Screen::displayMenu() {
  tft.fillRect(270, 40, 40, 190, BG_COLOR);

  // 1st item
  bool selected = app->currentScreen == SCREEN_CAR;
  int bgColor = selected ? 0x0000aa : BG_COLOR;

  tft.fillRoundRect(270, 40, 40, 40, 10, bgColor);
  tft.drawRoundRect(270, 40, 40, 40, 10, BORDER_COLOR);
  tft.fillCircle(290, 60, 13, BORDER_COLOR);
  tft.fillCircle(290, 60, 8, bgColor);
  tft.drawLine(280, 60, 300, 60, BORDER_COLOR);
  tft.drawLine(290, 50, 290, 70, BORDER_COLOR);
  tft.drawLine(285, 55, 295, 65, BORDER_COLOR);
  tft.drawLine(285, 65, 295, 55, BORDER_COLOR);

  // 2nd item
  selected = app->currentScreen == SCREEN_DHT;
  bgColor = selected ? 0x0000aa : BG_COLOR;

  tft.fillRoundRect(270, 90, 40, 40, 10, bgColor);
  tft.drawRoundRect(270, 90, 40, 40, 10, BORDER_COLOR);
  tft.drawRoundRect(285, 95, 10, 20, 5, BORDER_COLOR);
  tft.drawLine(285, 100, 289, 100, BORDER_COLOR);
  tft.drawLine(285, 105, 289, 105, BORDER_COLOR);
  tft.fillCircle(289, 118, 7, BORDER_COLOR);

  tft.setTextSize(2);
  tft.setTextColor(BORDER_COLOR);
  tft.fillRoundRect(270, 190, 40, 40, 10, app->joystickMode == HAND_MODE ? 0x0000aa : BG_COLOR);
  tft.drawRoundRect(270, 190, 40, 40, 10, BORDER_COLOR);
  tft.drawCentreString("H", 293, 195, 2);
  tft.setTextColor(0xffffff);
  tft.setTextSize(1);
}

void Screen::onNavClick(int y) {
  if (y < 85) {
    app->setScreen(SCREEN_CAR);
    return;
  }

  if (y < 135) {
    app->setScreen(SCREEN_DHT);
    return;
  }

  app->joystickMode = app->joystickMode == JOYSTICK_MODE ? HAND_MODE : JOYSTICK_MODE;
  displayMenu();
}

void Screen::updateDistance(int distance) {
  if (app->currentScreen != SCREEN_CAR)
    return;

  tft.fillRect(120, 200, 80, 20, BG_COLOR);

  if (distance > 40)
    return;

  uint8_t r = 255;
  uint8_t g = map(distance, 1, 40, 0, 255);
  uint8_t b = map(distance, 1, 40, 0, 255);

  uint16_t color = tft.color565(r, g, b);
  tft.setTextColor(color);

  char buffer[20];
  snprintf(buffer, sizeof(buffer), "%d cm", distance);

  tft.drawCentreString(buffer, 160, 200, 2);
  tft.setTextColor(0xffffff);
}
