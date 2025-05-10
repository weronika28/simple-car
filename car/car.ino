#include <WiFi.h>
#include <esp_now.h>
#include <RCCar.h>

#define ENA 23
#define IN1 12
#define IN2 14
#define IN3 18
#define IN4 19
#define ENB 25

// mac address of car: D0:EF:76:47:4D:28
uint8_t controllerAddress[] = { 0x78, 0x42, 0x1C, 0x65, 0x3D, 0x54 };

int left = 0, right = 0;
int lastRemoteUpdate = 0;

void setup() {
  Serial.begin(115200);
  initEspNow(controllerAddress);
  setEventHandler(&handleEvent);
  
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);
}

void loop() {
  if(millis() - lastRemoteUpdate > 1000)
    carStop();
}

void handleEvent(int event, char* message) {
  switch (event) {
    case EVENT_JOYSTICK:
      handleJoystickMovement(message);
      break;
    case EVENT_JOYSTICK_SW:
      handleJoystickButton(message);
      break;
  }
}

void handleJoystickMovement(char* message) {
  sscanf(message, "%d;%d", &left, &right);

  if(right < MIN_USABLE_SPEED && right > -MIN_USABLE_SPEED)
    right = 0;
  if(left < MIN_USABLE_SPEED && left > -MIN_USABLE_SPEED)
    left = 0;

  lastRemoteUpdate = millis();

  moveCar();  
}

void handleJoystickButton(char* message) {
  Serial.printf("button: %s", message);
}

void carStop() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void moveCar() {
  Serial.printf("Moving right %d, left %d\n", right, left);

  if (right == 0 && left == 0) {
    carStop();
    return;
  }

  // Left motor direction
  if (left >= 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  } else {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  }

  // Right motor direction
  if (right >= 0) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  } else {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }

  analogWrite(ENA, abs(left));
  analogWrite(ENB, abs(right));
}

