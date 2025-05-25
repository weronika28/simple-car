#include <Joystick.h>
#include <App.h>
#include <RCCar.h>
#include <Arduino.h>
#include <Adafruit_Sensor.h>

int loopTime = millis();

Joystick::Joystick(App* appParam) {
  this->app = appParam;
}

void Joystick::setup() {
  pinMode(JOY_SW_PIN, INPUT_PULLUP);
}

void Joystick::loop() {
  if (loopTime > millis())
    return;

  loopTime = millis() + 200;

  if (app->joystickMode == JOYSTICK_MODE)
    handleJoystick();
  else
    handleHandMode();
}

void Joystick::handleHandMode() {
  sensors_event_t a, g, temp;
  app->mpu->getEvent(&a, &g, &temp);

  int x = this->getNormalizedHandValue(a.acceleration.x, 1, 4); 
  int y = this->getNormalizedHandValue(-a.acceleration.y, 1, 5);

  if (abs(x) < 40)
    x = 0;
  if (x != 0)
    x += x < 0 ? 40 : -40;

  x *= ((float)MAX_SPEED / (MAX_SPEED - 40));

  Serial.printf("x:%d, y:%d\n", x, y);

  int right, left;

  int lx = abs(y) + x;
  int rx = abs(y) - x;

  if (y < 0) {
    lx *= -1;
    rx *= -1;
  }

  // Clamp to range -100 to 100
  left = max(-app->currentMaxSpeed, min(app->currentMaxSpeed, rx));
  right = max(-app->currentMaxSpeed, min(app->currentMaxSpeed, lx));

  app->setSpeed(left, right);
}

void Joystick::handleJoystick() {
  bool joy_sw = digitalRead(JOY_SW_PIN) == LOW;

  // we send joy sw status when its clicked or has changed
  if (joy_sw || joy_sw != last_sw) {
    last_sw = joy_sw;
    onJoyButtonPress();
  }

  int x = this->getNormalizedJoyValue(analogRead(JOY_X_PIN));
  int y = this->getNormalizedJoyValue(analogRead(JOY_Y_PIN));

  Serial.printf("x:%d, y:%d\n", x, y);

  int right, left;

  int lx = abs(x) - y;
  int rx = abs(x) + y;

  if (x < 0) {
    lx *= -1;
    rx *= -1;
  }

  // Clamp to range -100 to 100
  left = max(-app->currentMaxSpeed, min(app->currentMaxSpeed, rx));
  right = max(-app->currentMaxSpeed, min(app->currentMaxSpeed, lx));

  app->setSpeed(left, right);
}

void Joystick::onJoyButtonPress() {
  char* value = last_sw ? (char*)"1" : (char*)"0";
  sendEvent(EVENT_JOYSTICK_SW, value);
}

int16_t Joystick::getNormalizedJoyValue(int value) {
  int offset = value - 1830;  // 1850 is the middle value
  int16_t translatedValue = 0;

  if (abs(offset) < 60)
    return 0;

  // Limit offset to prevent out of bounds
  if (offset > 0) {
    // Exponential scaling for positive values (towards 100)
    translatedValue = (int16_t)(app->currentMaxSpeed * ((float)offset / (4080 - 1830)));
  } else if (offset < 0) {
    // Exponential scaling for negative values (towards -100)
    translatedValue = (int16_t)(-app->currentMaxSpeed * ((float)(-offset) / 1830));
  }

  // Return the translated value
  return translatedValue;
}

int16_t Joystick::getNormalizedHandValue(float value, float min, float max) {
  if (abs(value) < min)
    return 0;

  if (value < -max)
    value = -max;
  if (value > max)
    value = max;

  // Limit offset to prevent out of bounds
  if (value > 0) {
    // Exponential scaling for positive values (towards 100)
    return (int16_t)((app->currentMaxSpeed) * ((float)value / max));  // Exponential curve
  } else if (value < 0) {
    // Exponential scaling for negative values (towards -100)
    return (int16_t)(-app->currentMaxSpeed * ((float)(-value) / max));  // Exponential curve
  }

  // Return the translated value
  return 0;
}