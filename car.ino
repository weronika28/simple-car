#define ENA 23
#define IN1 18
#define IN2 19
#define IN3 21
#define IN4 22
#define ENB 25

int Speed = 150;
bool executed = false;

void setup() {
  Serial.begin(9600);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);
}

void loop() {
  if (!executed) {
    carforward();
    Serial.println("Forward");
    delay(700);

    carturnright();
    Serial.println("Turn right");
    delay(1700);

    carforward();
    Serial.println("Forward");
    delay(600);

    carStop();
    Serial.println("Stop");

    executed = true; 
  }
}

void carforward() {
  analogWrite(ENA, Speed);
  analogWrite(ENB, Speed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void carturnright() {
  analogWrite(ENA, Speed);
  analogWrite(ENB, Speed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void carStop() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}


// #define BLYNK_TEMPLATE_ID ""
// #define BLYNK_TEMPLATE_NAME "Car"
// #define BLYNK_AUTH_TOKEN "s"

// #include <WiFi.h>
// #include <BlynkSimpleEsp32.h>

// #define ENA 23    
// #define IN1 18    
// #define IN2 19    
// #define IN3 21   
// #define IN4 22    
// #define ENB 25 

// bool forward = 0;
// bool backward = 0;
// bool left = 0;
// bool right = 0;
// int Speed;

// char ssid[] = ""; // WiFi nazwa
// char pass[] = ""; // WiFi hasło

// void setup() {
//   Serial.begin(9600);
  
//   pinMode(ENA, OUTPUT);
//   pinMode(IN1, OUTPUT);
//   pinMode(IN2, OUTPUT);
//   pinMode(IN3, OUTPUT);
//   pinMode(IN4, OUTPUT);
//   pinMode(ENB, OUTPUT);

//   Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

// }

// BLYNK_WRITE(V0) { forward = param.asInt(); }
// BLYNK_WRITE(V1) { backward = param.asInt(); }
// BLYNK_WRITE(V2) { left = param.asInt(); }
// BLYNK_WRITE(V3) { right = param.asInt(); }
// BLYNK_WRITE(V4) { Speed = param.asInt(); }

// void loop() {
//   Blynk.run();
//   smartcar();
// }

// void smartcar() {
//   if (forward == 1) {
//     carforward();
//     Serial.println("carforward");
//   } else if (backward == 1) {
//     carbackward();
//     Serial.println("carbackward");
//   } else if (left == 1) {
//     carturnleft();
//     Serial.println("carleft");
//   } else if (right == 1) {
//     carturnright();
//     Serial.println("carright");
//   } else {
//     carStop();
//     Serial.println("carstop");
//   }
// }

// void carforward() {
//   analogWrite(ENA, Speed);
//   analogWrite(ENB, Speed);
//   digitalWrite(IN1, LOW);
//   digitalWrite(IN2, HIGH);
//   digitalWrite(IN3, HIGH);
//   digitalWrite(IN4, LOW);
// }

// void carbackward() {
//   analogWrite(ENA, Speed);
//   analogWrite(ENB, Speed);
//   digitalWrite(IN1, HIGH);
//   digitalWrite(IN2, LOW);
//   digitalWrite(IN3, LOW);
//   digitalWrite(IN4, HIGH);
// }

// void carturnleft() {
//   analogWrite(ENA, Speed);
//   analogWrite(ENB, Speed);
//   digitalWrite(IN1, HIGH);
//   digitalWrite(IN2, LOW);
//   digitalWrite(IN3, HIGH);
//   digitalWrite(IN4, LOW);
// }

// void carturnright() {
//   analogWrite(ENA, Speed);
//   analogWrite(ENB, Speed);
//   digitalWrite(IN1, LOW);
//   digitalWrite(IN2, HIGH);
//   digitalWrite(IN3, LOW);
//   digitalWrite(IN4, HIGH);
// }

// void carStop() {
//   digitalWrite(IN1, LOW);
//   digitalWrite(IN2, LOW);
//   digitalWrite(IN3, LOW);
//   digitalWrite(IN4, LOW);
// }
