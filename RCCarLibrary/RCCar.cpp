#include <Arduino.h>
#include "RCCar.h"
#include <WiFi.h>
#include <esp_now.h>

char incomingMessage[250];
uint8_t * address;
static EventHandler eventHandler = nullptr;
static MessageStatusHandler messageStatusHandler = nullptr;

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  static char buffer[250];
  if (len >= sizeof(buffer)) return;

  memcpy(buffer, incomingData, len);
  buffer[len] = '\0';

  char *separator = strchr(buffer, ';');
  if (separator != nullptr) {
    *separator = '\0';
    int event = atoi(buffer);
    char *message = separator + 1;
    internalHandleEvent(event, message);
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");

  if (messageStatusHandler) {
    messageStatusHandler(status != ESP_NOW_SEND_SUCCESS);
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
  char buffer[250];
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