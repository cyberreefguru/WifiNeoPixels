/*
 * WifiWrapper.h
 *
 *  Created on: Feb 22, 2016
 *      Author: tsasala
 */

#ifndef WIFIWRAPPER_H_
#define WIFIWRAPPER_H_

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "ClientGlobal.h"
#include "Configuration.h"
#include "Helper.h"

class WifiWrapper
{
public:
	WifiWrapper();
	uint8_t initialize(Configuration* config);
	uint8_t initialize();

	void waitForConfig();
	WiFiClient& getWifiClient();

protected:
	WiFiClient wifi;
	Configuration* config;

};

#endif /* WIFIWRAPPER_H_ */
