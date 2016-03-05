// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _WifiNeoPixelClient_H_
#define _WifiNeoPixelClient_H_
#include <Arduino.h>

//#include <ESP8266WiFi.h>
//#include <PubSubClient.h>

#include "Command.h"
//#include "NeoPixelWrapper.h"
//#include "PubSubWrapper.h"
//#include "WifiWrapper.h"
#include "Configuration.h"
#include "Helper.h"
#include "Menu.h"


#define NUM_PIXELS 4
#define HALF NUM_PIXELS/2

void worker();

boolean isCommandAvailable();
void setCommandAvailable(boolean flag);
uint8_t commandDelay(uint32_t time);

void pubsubCallback(char* topic, byte* payload, unsigned int length);


#endif /* _WifiNeoPixelClient_H_ */
