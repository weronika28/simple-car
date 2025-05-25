#include <RCCar.h>
#include <App.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <esp_now.h>

#define MPU_SDA 21
#define MPU_SCL 22

// mac address of controller: 00:4B:12:53:2C:E8
uint8_t carAddress[] = { 0xD0, 0xEF, 0x76, 0x47, 0x4D, 0x28 };

App::App() {
  screen = new Screen(this);
  joystick = new Joystick(this);
  mpu = new Adafruit_MPU6050();
}

void App::setup() {
  Serial.begin(115200);
  initEspNow(carAddress);

  joystick->setup();
  screen->setup();

  setScreen(SCREEN_CAR);
  Wire.begin(MPU_SDA, MPU_SCL);

  if (!mpu->begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  mpu->setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu->setGyroRange(MPU6050_RANGE_250_DEG);
  mpu->setFilterBandwidth(MPU6050_BAND_5_HZ);

  Serial.println("MPU6050 initialized");
  delay(100);
}

void App::loop() {
  joystick->loop();
  screen->loop();
}

void App::setScreen(int newScreen) {
  this->currentScreen = newScreen;

  this->screen->onScreenChange();
}

void App::setCurrentMaxSpeed(int speed) {
  if (currentMaxSpeed == speed)
    return;

  currentMaxSpeed = speed;
}

void App::setSpeed(int left, int right) {
  if (abs(left) < MIN_USABLE_SPEED && abs(right) < MIN_USABLE_SPEED) {
    left = 0;
    right = 0;
  }

  char buffer[20];
  snprintf(buffer, sizeof(buffer), "%d;%d", left, right);
  sendEvent(EVENT_JOYSTICK, buffer);

  if (this->left == left && this->right == right)
    return;

  this->left = left;
  this->right = right;
  this->screen->updateSpeed();
}

void App::handleMessageStatus(bool error) {
  screen->setConnectionStatus(!error);
}

void App::handleEvent(int event, char* message) {
  if (event == EVENT_DHT) {
    sscanf(message, "%f;%f", &temperature, &humidity);
    this->lastDhtUpdate = millis();
    this->screen->updateTemperatureScreen();
  }

  if (event == EVENT_DISTANCE) {
    int distance;
    sscanf(message, "%d", &distance);
    this->screen->updateDistance(distance);
  }
}
