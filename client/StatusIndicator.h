/*
 * StatusIndicator.h
 *
 *  Created on: Sep 12, 2015
 *      Author: tsasala
 */

#ifndef STATUSINDICATOR_H_
#define STATUSINDICATOR_H_

#include <Arduino.h>
#include <FastLed.h>

#include "ClientGlobal.h"
#include "Helper.h"
#include "NeopixelWrapper.h"

#define STATUS_LED_NUM				5
#define STATUS_LED_PIN				5 // TODO set which pin!!

// Locations of specific LEDs within the array
//#define STATUS_CONFIG				0
//#define STATUS_QUEUE				1
//#define STATUS_WIFI					2
//#define STATUS_DRIVER				3
//#define STATUS_GENERAL				4

// Status colors associated with the general LED

#define STATUS_COLOR_UNKNOWN		GREY
#define	STATUS_COLOR_NONE			BLACK
#define STATUS_COLOR_BOOTING		ORANGE
#define	STATUS_COLOR_WAITING		GREEN
#define STATUS_COLOR_PROCESSING		BLUE

#define STATUS_COLOR_CONFIGURING	YELLOW
#define STATUS_COLOR_UPLOADING		MAGENTA

// Status colors associated with the specific LEDs
#define	STATUS_COLOR_OK				GREEN
#define STATUS_COLOR_ERROR			RED

#define STATUS_CONTROLLER			NEOPIXEL
#define STATUS_COLOR_ORDER			RGB
#define STATUS_COLOR_CORRECTION		TypicalLEDStrip
#define STATUS_DEFAULT_INTENSITY	25

// TODO - change status to enum

class StatusIndicator
{
public:
	StatusIndicator();

	uint8_t initialize();

	void setIntensity(uint8_t i);
	uint8_t getIntensity();

	void setStatuses(StatusEnum s);

	void setConfigStatus(StatusEnum s);
	void setQueueStatus(StatusEnum s);
	void setWifiStatus(StatusEnum s);
	void setDriverStatus(StatusEnum s);
	void setGeneralStatus(StatusEnum s);
	void setStatus(ComponentEnum comp, StatusEnum s);

protected:
	CRGB *statusLeds;
	uint8_t intensity;
	StatusEnum statuses[sizeof(ComponentEnum)];

	void show();

};

//end of add your includes here
#ifdef __cplusplus
extern "C"
{
#endif

extern uint8_t isCommandAvailable();
extern uint8_t commandDelay(uint32_t time);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* STATUSINDICATOR_H_ */
