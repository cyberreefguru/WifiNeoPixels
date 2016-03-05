/*
 * Menu.h
 *
 *  Created on: Feb 21, 2016
 *      Author: tsasala
 */

#ifndef MENU_H_
#define MENU_H_

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "Command.h"
#include "Configuration.h"

#include "PubSubWrapper.h"
#include "NeoPixelWrapper.h"
#include "WifiWrapper.h"
#include "Helper.h"

#define STATUS_NOP		0
#define STATUS_REBOOT	1

#define INPUT_BUFFER_SIZE		20


class Menu
{
public:
	Menu();
	uint8_t initialize(Configuration* config);
//	uint8_t initialize(Configuration* config, WifiWrapper* wifi, PubSubWrapper* pubsub, NeopixelWrapper* controller);
	uint8_t configure();
	void waitForConfig();

protected:
	Configuration* config;
//	WifiWrapper* wifi;
//	PubSubWrapper* pubsub;
//	NeopixelWrapper* controller;

	void readAll();

};

#endif /* MENU_H_ */
