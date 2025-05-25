#include <App.h>
#include <RCCar.h>

App app;

void setup() {
  randomSeed(analogRead(14));

  app.setup();

  setMessageStatusHandler(&handleMessageStatus);
  setEventHandler(&handleEvent);

  delay(200);
}

void loop() {
  loopEspNow();
  app.loop();
}

void handleMessageStatus(bool error) {
  app.handleMessageStatus(error);
}

void handleEvent(int event, char* message) {
  app.handleEvent(event, message);
}