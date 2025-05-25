#include <Screen.h>
#include <Joystick.h>
#include <Arduino.h>
#include <Adafruit_MPU6050.h>

#ifndef APP_H
#define APP_H

#define SCREEN_CAR 1
#define SCREEN_DHT 2

#define JOYSTICK_MODE 1
#define HAND_MODE 2

class App {
public:
  Screen* screen;
  Adafruit_MPU6050* mpu;
  
  int currentMaxSpeed = 200;
  int left = -1;
  int right = -1;
  float temperature = -1000;
  float humidity = -1000;
  int lastDhtUpdate = -1;
  int currentScreen;
  int joystickMode = JOYSTICK_MODE;

  App();
  void setup();
  void loop();
  void setScreen(int screen);
  void setSpeed(int left, int right);
  void handleMessageStatus(bool error);
  void handleEvent(int event, char* message);
  void setCurrentMaxSpeed(int speed);

private:
  Joystick* joystick;
};

#endif
