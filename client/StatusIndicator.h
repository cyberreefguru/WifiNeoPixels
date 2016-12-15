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

#define STATUS_CONFIG				0
#define STATUS_QUEUE				1
#define STATUS_WIFI					2
#define STATUS_DRIVER				3
#define STATUS_GENERAL				4


#define	STATUS_COLOR_NONE			BLACK
#define	STATUS_COLOR_OK				GREEN
#define STATUS_COLOR_FAIL			RED
#define STATUS_COLOR_UNKNOWN		CYAN
#define STATUS_COLOR_PROCESSING		BLUE
#define STATUS_COLOR_BOOTING		PINK


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

	void setStatus(CRGB status);

	void setConfigStatus(CRGB status);
	void setQueueStatus(CRGB status);
	void setWifiStatus(CRGB status);
	void setDriverStatus(CRGB status);
	void setGeneralStatus(CRGB status);

protected:
	CRGB *statusLeds;
	uint8_t intensity;

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
