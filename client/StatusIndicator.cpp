/*
 * StatusIndicator.cpp
 *
 *  Created on: Sep 12, 2015
 *      Author: tsasala
 */

#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_ESP8266_RAW_PIN_ORDER
//#define FASTLED_ESP8266_NODEMCU_PIN_ORDER

#include "StatusIndicator.h"

CLEDController *statusController;

/**
 * Constructor
 */
StatusIndicator::StatusIndicator()
{
	statusLeds = NULL;
	intensity = STATUS_DEFAULT_INTENSITY;
	setStatuses( Unknown );
}


/**
 * Initializes the library
 */
boolean StatusIndicator::initialize()
{
	boolean status = false;
	uint8_t i;

	// Free memory if we already allocated it
	if( statusLeds != 0 )
	{
		free(statusLeds);
	}

	// Allocate memory for LED buffer
	statusLeds = (CRGB *) malloc(sizeof(CRGB) * STATUS_LED_NUM);
	if (statusLeds == 0)
	{
		Serial.println(F("ERROR - unable to allocate status LED memory"));
	}
	else
	{
		statusController = &FastLED.addLeds<STATUS_CONTROLLER, STATUS_LED_PIN>(statusLeds, STATUS_LED_NUM).setCorrection(STATUS_COLOR_CORRECTION); // strip
		statusController->clearLedData();
		Helper::workYield();

		for(i=0; i<STATUS_LED_NUM; i++)
		{
			statusLeds[i] = STATUS_COLOR_PROCESSING;
			statusController->showLeds(intensity);
			delay(350);
		}
		statusController->clearLedData();
		statusController->showLeds(intensity);
		Helper::workYield();

		status = true;
	}

	return status;
}

/**
 * Returns the hue update time
 */
uint8_t StatusIndicator::getIntensity()
{
	return intensity;
}

/**
 * Changes the amount of time to wait before updating the hue
 *
 */
void StatusIndicator::setIntensity(uint8_t i)
{
	intensity = i;
}

void StatusIndicator::show()
{
	statusController->showLeds(intensity);
}

/**
 * Sets status of all indicators
 */
void StatusIndicator::setStatuses(StatusEnum s)
{
	uint8_t i;

	for(i=0; i<sizeof(ComponentEnum); i++)
	{
		statuses[i] = s;
	}
}

void StatusIndicator::setConfigStatus(StatusEnum s)
{
	setStatus(Config, s);
}

void StatusIndicator::setQueueStatus(StatusEnum s)
{
	setStatus(Queue, s);
}

void StatusIndicator::setWifiStatus(StatusEnum s)
{
	setStatus(Wifi, s);
}

void StatusIndicator::setDriverStatus(StatusEnum s)
{
	setStatus(Driver, s);
}

void StatusIndicator::setGeneralStatus(StatusEnum s)
{
	setStatus(General, s);
}

void StatusIndicator::setStatus(ComponentEnum comp, StatusEnum s)
{
	switch(s)
	{
	case Unknown:
		statusLeds[comp] = STATUS_COLOR_UNKNOWN;
		break;
	case None:
		statusLeds[comp] = STATUS_COLOR_NONE;
		break;
	case Ok:
		statusLeds[comp] = STATUS_COLOR_OK;
		break;
	case Waiting:
		statusLeds[comp] = STATUS_COLOR_WAITING;
		break;
	case Processing:
		statusLeds[comp] = STATUS_COLOR_PROCESSING;
		break;
	case Configuring:
		statusLeds[comp] = STATUS_COLOR_CONFIGURING;
		break;
	case Booting:
		statusLeds[comp] = STATUS_COLOR_BOOTING;
		break;
	case Uploading:
		statusLeds[comp] = STATUS_COLOR_UPLOADING;
		break;
	case Error:
		statusLeds[comp] = STATUS_COLOR_ERROR;
		break;
	}
	statusController->showLeds(intensity);
}
