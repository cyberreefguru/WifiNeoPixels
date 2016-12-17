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
	char hostname[20];

	Serial.println(F("Initializing WIFI..."));

	if( config == NULL )
	{
		Serial.println(F("ERROR - configuration object not defined."));
		return flag;
	}

	Serial.print(F("Connection Status: "));
	if( WiFi.status() == WL_CONNECTED )
	{
		Serial.println( F("CONNECTED") );
	}
	else if( WiFi.status() == WL_DISCONNECTED)
	{
		Serial.println( F("DISCONNECTED") );
	}
	else if( WiFi.status() == WL_CONNECTION_LOST)
	{
		Serial.println( F("LOST") );
	}
	else if( WiFi.status() == WL_CONNECT_FAILED)
	{
		Serial.println( F("FAILED") );
	}
	else if( WiFi.status() == WL_SCAN_COMPLETED)
	{
		Serial.println( F("SCAN COMPLETE") );
	}
	else if( WiFi.status() == WL_NO_SSID_AVAIL)
	{
		Serial.println( F("NO SSID") );
	}
	else if( WiFi.status() == WL_IDLE_STATUS)
	{
		Serial.println( F("IDLE") );
	}
	else if( WiFi.status() == WL_NO_SHIELD)
	{
		Serial.println( F("NO SHIELD") );
	}

	// If connected, reset connection
	if( WiFi.status() == WL_CONNECTED )
	{
		Serial.println(F("Disconnecting current session..."));
		WiFi.disconnect(1);
		Helper::workYield(); // Delay and give time to ESP
	}

	// Connect to specified network
	Serial.print(F("Connecting to "));
	Serial.print((char *)config->getSsid() );

	// We start by connecting to a WiFi network
	WiFi.begin( (char *)config->getSsid(), (char *)config->getPassword());

	Helper::delayYield(250); // Give time to ESP
	//delay(250);

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
			Serial.print(F("."));
			Helper::delayYield(500); // Delay and give time to ESP
		}
	}

	if( flag )
	{
		Serial.print(F(".SUCCESS!\n\rConnected: "));
		Serial.println(WiFi.localIP());

		// pause after configuring WIFI
		Helper::delayYield(250); // Give time to ESP

		Serial.println(F("Setting up OTA..."));

		sprintf(hostname, "lednode-%i", config->getNodeId() );

		ArduinoOTA.setHostname(hostname);

		ArduinoOTA.onStart([]() {
			Serial.println("OTA Start");
		});

		ArduinoOTA.onEnd([]() {
			Serial.println("OTA End");
			Serial.println("Rebooting...");
		});

		ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
			Serial.printf("Progress: %u%%\r\n", (progress / (total / 100)));
		});

		ArduinoOTA.onError([](ota_error_t error) {
			Serial.printf("Error[%u]: ", error);
			if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
			else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
			else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
			else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
			else if (error == OTA_END_ERROR) Serial.println("End Failed");
		});
		ArduinoOTA.begin();

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

uint8_t WifiWrapper::connected()
{
	if( WiFi.status() == WL_CONNECTED )
	{
		return true;
	}
	else
	{
		return false;
	}
}

void WifiWrapper::work()
{
	  ArduinoOTA.handle();
}
