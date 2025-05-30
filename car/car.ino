#include <WiFi.h>
#include <esp_now.h>
#include <RCCar.h>
#include <DHT.h>
#include <NewPing.h>

#define MAX_DISTANCE 200
#define TRIG_PIN 32
#define ECHO_PIN 33

#define ENA 23
#define IN1 12
#define IN2 14
#define IN3 18
#define IN4 19
#define ENB 25

#define DHT_PIN 4
#define DHT_TYPE DHT11
#define BUZZER_PIN 26
#define LED_DRL_PIN 2
#define LED_LEFT_TURN_PIN 13
#define LED_RIGHT_TURN_PIN 5


NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);
DHT dht(DHT_PIN, DHT_TYPE);

// mac address of car: D0:EF:76:47:4D:28
uint8_t controllerAddress[] = { 0x00, 0x4B, 0x12, 0x53, 0x2C, 0xE8 };

int left = 0, right = 0;
int lastRemoteUpdate = 0;
bool leftTurn = false;
bool rightTurn = false;
unsigned long lastBlinkTime = 0;
bool blinkState = false;
long duration;

bool hornActive = false;

float lastDistance = 0;

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

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_LEFT_TURN_PIN, OUTPUT);
  pinMode(LED_RIGHT_TURN_PIN, OUTPUT);
  pinMode(LED_DRL_PIN, OUTPUT);

  digitalWrite(LED_DRL_PIN, HIGH);
  digitalWrite(LED_LEFT_TURN_PIN, LOW);
  digitalWrite(LED_RIGHT_TURN_PIN, LOW);

  dht.begin();
}

void loop() {
  loopEspNow();
  if (millis() - lastRemoteUpdate > 1000) {
    carStop();
    noTone(BUZZER_PIN);
    hornActive = false;
  }

  static unsigned long lastDhtRead = 0;
  if (millis() - lastDhtRead > 5000) {
    lastDhtRead = millis();
    readDHTData();
  }

  static unsigned long lastDistanceCheck = 0;
  if (millis() - lastDistanceCheck > 200) {
    lastDistanceCheck = millis();
    checkDistance();
  }

  updateTurnSignals();
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

  if (right < MIN_USABLE_SPEED && right > -MIN_USABLE_SPEED)
    right = 0;
  if (left < MIN_USABLE_SPEED && left > -MIN_USABLE_SPEED)
    left = 0;

  lastRemoteUpdate = millis();

  moveCar();
}

void handleJoystickButton(char* message) {
  if (strcmp(message, "1") == 0) {
    if (!hornActive)
      tone(BUZZER_PIN, 1000);
  } else {
    if (hornActive)
      noTone(BUZZER_PIN);
  }

  hornActive = strcmp(message, "1") == 0;
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
  if (right == 0 && left == 0) {
    carStop();
    return;
  }

  Serial.printf("Moving right %d, left %d\n", right, left);

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

void readDHTData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  char buffer[250];
  Serial.printf("%.2f;%.2f\n", temperature, humidity);
  snprintf(buffer, sizeof(buffer), "%.2f;%.2f", temperature, humidity);

  sendEvent(EVENT_DHT, buffer);
}

void testBuzzer() {
  tone(BUZZER_PIN, 1000);
  delay(500);
  noTone(BUZZER_PIN);
  delay(1000);
}

float readDistanceCM() {
  unsigned int distance = sonar.ping_cm();
  if (distance == 0) return -1;
  return (float)distance;
}

void checkDistance() {
  float distance = readDistanceCM();
  if (distance <= 0) {
    return;
  }

  Serial.print("Odległość: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance != lastDistance) {
    String distStr = String((int)distance);
    sendEvent(EVENT_DISTANCE, (char*)distStr.c_str());
  }

  lastDistance = distance;
  if (distance > 40 && !hornActive) {
    noTone(BUZZER_PIN);
    return;
  }

  int delayTime = map((int)distance, 2, 40, 100, 600);

  tone(BUZZER_PIN, hornActive ? 1200 : 1000);
  delay(100);
  if (hornActive) {
    tone(BUZZER_PIN, 1000);
  } else {
    noTone(BUZZER_PIN);
  }
  delay(delayTime);
}

void updateTurnSignals() {
  if (abs(left) > abs(right) + 100) {
    rightTurn = true;
    leftTurn = false;
  } else if (abs(right) > abs(left) + 100) {
    leftTurn = true;
    rightTurn = false;
  } else {
    leftTurn = false;
    rightTurn = false;
  }

  if (millis() - lastBlinkTime >= 200) {
    lastBlinkTime = millis();
    blinkState = !blinkState;

    digitalWrite(LED_LEFT_TURN_PIN, (leftTurn && blinkState) ? HIGH : LOW);
    digitalWrite(LED_RIGHT_TURN_PIN, (rightTurn && blinkState) ? HIGH : LOW);
  }

  if (!leftTurn) digitalWrite(LED_LEFT_TURN_PIN, LOW);
  if (!rightTurn) digitalWrite(LED_RIGHT_TURN_PIN, LOW);
}
