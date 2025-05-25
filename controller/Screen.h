#include <Arduino.h>

#ifndef SCREEN_H
#define SCREEN_H

class App;

class Screen {
public:
  Screen(App* app);
  void setup();
  void loop();
  void updateSpeed();
  void setConnectionStatus(bool connected);
  void onScreenChange();
  void updateDistance(int distance);
  void updateTemperatureScreen();
  void updateTemperatureUpdateTime();

private:
  App* app;
  bool connected = true;

  void touchLoop();
  void handleTouch(int x, int y);
  void displayCar();
  void displayMenu();
  void onSpeedBarClick(int y);
  void updateSpeedBar();
  void displaySpeedBar();
  void onNavClick(int y);
  int calculateNextAnimationFrame(int current, int target, int divider, int min);
};

#endif
