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
#include <ArduinoOTA.h>


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
	uint8_t connected();
	void work();

protected:
	WiFiClient wifi;
	Configuration* config;

};


//end of add your includes here
#ifdef __cplusplus
extern "C"
{
#endif

extern void setStatus(volatile StatusEnum status);
extern volatile StatusEnum getStatus();

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* WIFIWRAPPER_H_ */
