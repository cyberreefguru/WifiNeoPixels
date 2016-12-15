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

void StatusIndicator::setStatus(CRGB b)
{
	uint8_t i;

	for(i=0; i<STATUS_LED_NUM; i++)
	{
		statusLeds[i] = b;
	}
	show();
}

void StatusIndicator::setConfigStatus(CRGB b)
{
	statusLeds[STATUS_CONFIG] = b;
	show();
}

void StatusIndicator::setQueueStatus(CRGB b)
{
	statusLeds[STATUS_QUEUE] = b;
	show();
}

void StatusIndicator::setWifiStatus(CRGB b)
{
	statusLeds[STATUS_WIFI] = b;
	show();
}

void StatusIndicator::setDriverStatus(CRGB b)
{
	statusLeds[STATUS_DRIVER] = b;
	show();
}

void StatusIndicator::setGeneralStatus(CRGB b)
{
	statusLeds[STATUS_GENERAL] = b;
	show();
}
