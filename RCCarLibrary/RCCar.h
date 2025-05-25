#include <Arduino.h>

#ifndef SHARED_H
#define SHARED_H
#pragma once

#define EVENT_JOYSTICK 1
#define EVENT_JOYSTICK_SW 2 
#define EVENT_DHT 3 
#define EVENT_DISTANCE 4

#define MIN_USABLE_SPEED 70
#define MIN_SPEED 100
#define MAX_SPEED 251
 
typedef void (*EventHandler)(int event, char* message);
typedef void (*MessageStatusHandler)(bool error);

void initEspNow(uint8_t *peerAddress);
void sendEvent(int event, char* message);
void setEventHandler(EventHandler handler);
void setMessageStatusHandler(MessageStatusHandler handler);
void internalHandleEvent(int event, char* message);
void loopEspNow();

#endif