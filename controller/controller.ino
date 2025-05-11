#include <LiquidCrystal.h>
#include <WiFi.h>
#include <esp_now.h>
#include <RCCar.h>

#define JOY_SW_PIN 21
#define JOY_X_PIN 34
#define JOY_Y_PIN 35

void onJoyButtonPress();

// mac address of controller: 78:42:1C:65:3D:54
uint8_t carAddress[] = { 0xD0, 0xEF, 0x76, 0x47, 0x4D, 0x28 };
bool last_sw = false;
int currentMaxSpeed = 160;  // default - each button press += 30 but keeps between MIN_SPEED and MAX_SPEED
bool lastError = true, error = false;

LiquidCrystal lcd(32, 14, 27, 26, 25, 33); // RS, E, D4, D5, D6, D7

void setup() {
  Serial.begin(115200);
  initEspNow(carAddress);
  setMessageStatusHandler(&handleMessageStatus);

  pinMode(JOY_SW_PIN, INPUT_PULLUP);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Connected: ...");

  displayMaxSpeed();
}

void loop() {
  delay(200);
  handleJoystick();
  displayConnectionStatus();
}

void handleMessageStatus(bool e) {
  // set error to new value - we will update this in loop() not here because handleMessageStatus is called asynchronously
  error = e;
}

void handleJoystick() {
  bool joy_sw = digitalRead(JOY_SW_PIN) == LOW;

  if (joy_sw != last_sw) {
    last_sw = joy_sw;
    char* value = last_sw ? (char*)"1" : (char*)"0";
    sendEvent(EVENT_JOYSTICK_SW, value);  // we send button click to the car but for now it not used anywhere

    if (joy_sw)
      onJoyButtonPress();
  }

  int x = getNormalizedJoyValue(analogRead(JOY_X_PIN));
  int y = getNormalizedJoyValue(analogRead(JOY_Y_PIN));

  int right, left;

  int lx = abs(y) + x;
  int rx = abs(y) - x;

  if (y < 0) {
    lx *= -1;
    rx *= -1;
  }

  left = max(-currentMaxSpeed, min(currentMaxSpeed, rx));
  right = max(-currentMaxSpeed, min(currentMaxSpeed, lx));

  if(left > -MIN_USABLE_SPEED && left < MIN_USABLE_SPEED)
    left = 0;
  if(right > -MIN_USABLE_SPEED && right < MIN_USABLE_SPEED)
    right = 0;

  char buffer[64];

  snprintf(buffer, sizeof(buffer), "%d;%d", left, right);
  sendEvent(EVENT_JOYSTICK, buffer);

  displayCurrentSpeed(left, right);
}

void onJoyButtonPress() {
  currentMaxSpeed += 30;

  if (currentMaxSpeed > MAX_SPEED)
    currentMaxSpeed = MIN_SPEED;
  Serial.printf("Changed car speed to %d\n", currentMaxSpeed);
  displayMaxSpeed();
}

void displayConnectionStatus() {
  if(lastError == error)
    return;

  lastError = error;

  lcd.setCursor(11, 0);
  lcd.print(error ? "no " : "yes");
}

void displayMaxSpeed() {
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "m:%d", currentMaxSpeed);
  lcd.setCursor(11, 1);
  lcd.print(buffer);
}

void displayCurrentSpeed(int left, int right) {
  lcd.setCursor(0, 1);
  lcd.print("         ");

  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%d/%d", left, right);
  lcd.setCursor(0, 1);
  lcd.print(buffer);
}

int16_t getNormalizedJoyValue(int value) {
  int offset = value - 1830;  // 1830 is the middle value
  int16_t translatedValue = 0;

  if (abs(offset) < 60)
    return 0;

  if (offset > 0) {
    // Scaling for positive values (towards currentMaxSpeed)
    translatedValue = (int16_t)(currentMaxSpeed * ((float)offset / (4095 - 1830))); 
  } else if (offset < 0) {
    // Scaling for negative values (towards -currentMaxSpeed)
    translatedValue = (int16_t)(-currentMaxSpeed * ((float)(-offset) / 1830));
  }

  // Return the translated value
  return translatedValue;
}
