/*
 * WifiWrapper.cpp
 *
 *  Created on: Feb 22, 2016
 *      Author: tsasala
 */

#include "WifiWrapper.h"

WifiWrapper::WifiWrapper()
{
	config = 0;
}


/**
 * Initializes WIFI on the ESP8266
 *
 */
uint8_t WifiWrapper::initialize(Configuration* config)
{
	this->config = config;
	return initialize();
}


/**
 * Initializes the WIFI module
 *
 * The method allows initialize to be called without access to the config object.
 *
 */
uint8_t WifiWrapper::initialize()
{
	uint8_t flag = false;
	uint8_t count = 0;

	Serial.println(F("Initializing WIFI..."));

	Serial.print(F("Connection Status: "));
	Serial.println( WiFi.status() );

	delay(500);
	if( WiFi.status() == WL_CONNECTED )
	{
		Serial.println(F("Disconnecting current session..."));
		WiFi.disconnect();
	}

	Serial.print(F("Connecting to "));
	Serial.print((char *)config->getSsid() );

	// We start by connecting to a WiFi network
	WiFi.begin( (char *)config->getSsid(), (char *)config->getPassword());

	while( count < config->getWifiTries() )
	{
		if( WiFi.status() == WL_CONNECTED )
		{
			flag = true;
			break;
		}
		else
		{
			count += 1;
			Serial.print(".");
			delay(500);
		}
	}

	if( flag )
	{
		Serial.print(F(".SUCCESS!\n\rConnected: "));
		Serial.println(WiFi.localIP());
	}
	else
	{
		Serial.print(F(".FAILED!"));
	}

	return flag;

}

/**
 * Returns the WIFI Client
 *
 */
WiFiClient& WifiWrapper::getWifiClient()
{
	return wifi;
}
