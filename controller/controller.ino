#include <WiFi.h>
#include <esp_now.h>
#include <RCCar.h>

#define JOY_SW_PIN 32
#define JOY_X_PIN 34
#define JOY_Y_PIN 35

TFT_eSPI tft = TFT_eSPI();

void onJoyButtonPress();

// mac address of controller: 78:42:1C:65:3D:54
uint8_t carAddress[] = { 0xD0, 0xEF, 0x76, 0x47, 0x4D, 0x28 };
bool last_sw = false;
int currentMaxSpeed = 160; // default - each button press += 30 but keeps between MIN_SPEED and MAX_SPEED

void setup() {
  Serial.begin(115200);
  initEspNow(carAddress);

  pinMode(JOY_SW_PIN, INPUT_PULLUP);
  
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Hello, HX8347G!");
}

void loop() {
  delay(200);
  //handleJoystick();
}

void handleJoystick() {
  bool joy_sw = digitalRead(JOY_SW_PIN) == LOW;

  if (joy_sw != last_sw) {
    last_sw = joy_sw;
    char* value = last_sw ? (char*)"1" : (char*)"0";
    sendEvent(EVENT_JOYSTICK_SW, value); // we send button click to the car but for now it not used anywhere

    if(joy_sw)
      onJoyButtonPress();
  }

  int x = getNormalizedJoyValue(analogRead(JOY_X_PIN));
  int y = getNormalizedJoyValue(analogRead(JOY_Y_PIN));

  int right, left;

  int lx = abs(y) + x;
  int rx = abs(y) - x;

  if(y<0) {
    lx *= -1;
    rx *= -1;
  }

  // Clamp to range -100 to 100
  left = max(-currentMaxSpeed, min(currentMaxSpeed, rx));
  right = max(-currentMaxSpeed, min(currentMaxSpeed, lx));

  char buffer[250];
  snprintf(buffer, sizeof(buffer), "%d;%d", left, right);
  sendEvent(EVENT_JOYSTICK, buffer);
}

void onJoyButtonPress() {
  currentMaxSpeed += 30;

  if(currentMaxSpeed > MAX_SPEED)
    currentMaxSpeed = MIN_SPEED;
  Serial.printf("Changed car speed to %d\n", currentMaxSpeed);
}

int16_t getNormalizedJoyValue(int value) {
  int offset = value - 1830;  // 1850 is the middle value
  int16_t translatedValue = 0;

  if (abs(offset) < 60)
    return 0;

  // Limit offset to prevent out of bounds
  if (offset > 0) {
    // Exponential scaling for positive values (towards 100)
    translatedValue = (int16_t)(currentMaxSpeed * ((float)offset / (4096 - 1830)));  // Exponential curve
  } else if (offset < 0) {
    // Exponential scaling for negative values (towards -100)
    translatedValue = (int16_t)(-currentMaxSpeed * ((float)(-offset) / 1830));  // Exponential curve
  }

  // Return the translated value
  return translatedValue;
}
