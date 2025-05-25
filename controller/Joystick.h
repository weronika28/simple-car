#include <Arduino.h>

#ifndef JOYSTICK_H
#define JOYSTICK_H

#define JOY_SW_PIN 33
#define JOY_X_PIN 34
#define JOY_Y_PIN 32

class App;

class Joystick {
public:
  Joystick(App* appParam);
  void setup();
  void loop();

private:
  App* app;
  bool last_sw = false;
  int16_t getNormalizedJoyValue(int value);
  int16_t getNormalizedHandValue(float value, float min, float max);
  void onJoyButtonPress();
  void handleJoystick();
  void handleHandMode();
};


#endif
