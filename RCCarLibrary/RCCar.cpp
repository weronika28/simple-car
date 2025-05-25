#include <Arduino.h>
#include "RCCar.h"
#include <WiFi.h>
#include <esp_now.h>

#define MAX_MESSAGES 10
#define MAX_MESSAGE_SIZE 250

int loopTimer = 0;

uint8_t messageList[MAX_MESSAGES][MAX_MESSAGE_SIZE];
int messageLengths[MAX_MESSAGES];
int head = 0;
int tail = 0;

int sendStatus = 0;

uint8_t * address;
static EventHandler eventHandler = nullptr;
static MessageStatusHandler messageStatusHandler = nullptr;

bool shouldIgnore() {
  return millis() < 2000;
}

bool isBufferEmpty() {
  return head == tail;
}

bool isBufferFull() {
  return ((head + 1) % MAX_MESSAGES) == tail;
}

void addMessage(const uint8_t* data, int len) {
  if (isBufferFull()) return;
  if (len > MAX_MESSAGE_SIZE) len = MAX_MESSAGE_SIZE;
  
  memcpy(messageList[head], data, len);
  messageLengths[head] = len;
  head = (head + 1) % MAX_MESSAGES;
}

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  addMessage(incomingData, len);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (messageStatusHandler) {
    sendStatus = status != ESP_NOW_SEND_SUCCESS ? 2 : 1;
  }
}

void setEventHandler(EventHandler handler) {
  eventHandler = handler;
}

void setMessageStatusHandler(MessageStatusHandler handler) {
  messageStatusHandler = handler;
}

void initEspNow(uint8_t *peerAddress) {
  address = peerAddress;
  WiFi.mode(WIFI_STA);
  delay(100);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;

  if (!esp_now_is_peer_exist(peerAddress)) {
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Failed to add peer");
      return;
    }
  }

  Serial.println("Connected succesfully");
}

void sendEvent(int event, char* message) {
  if(shouldIgnore()) return;

  char buffer[30];
  snprintf(buffer, sizeof(buffer), "%d;%s", event, message);
  esp_now_send(address, (uint8_t*)buffer, strlen(buffer));
}

void internalHandleEvent(int event, char* message) {
  Serial.print("event: ");
  Serial.print(event);
  Serial.print(", message: ");
  Serial.println(message);

  if (eventHandler) {
    eventHandler(event, message);
  }
}

void handleMessage(const uint8_t* data, int len) {
  static char buffer[250];
  if (len >= sizeof(buffer)) return;

  memcpy(buffer, data, len);
  buffer[len] = '\0';

  char *separator = strchr(buffer, ';');
  if (separator != nullptr) {
    *separator = '\0';
    int event = atoi(buffer);
    char *message = separator + 1;
    internalHandleEvent(event, message);
  }
}

void loopEspNow() {
  if(shouldIgnore()) return;

  if(loopTimer > millis())
    return;
  loopTimer = millis() + 100;

  if(sendStatus != 0) {
    messageStatusHandler(sendStatus == 2);
  }

  sendStatus = 0;

  while (!isBufferEmpty()) {
    int index = tail;
    handleMessage(messageList[index], messageLengths[index]);
    tail = (tail + 1) % MAX_MESSAGES;
  }
}