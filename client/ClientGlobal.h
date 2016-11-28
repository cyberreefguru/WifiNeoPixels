/*
 * Global.h
 *
 *  Created on: Oct 30, 2016
 *      Author: tsasala
 */

#ifndef __CLIENT_GLOBAL_H_
#define __CLIENT_GLOBAL_H_

#define __MY_HOST 		NODEMCU
//#define __MY_HOST 	WITTY
//#define __MY_HOST 	ARDUINO

#define MY_LEDS		STRING
//#define MY_LEDS		STRIP

#define MY_CONTROLLER	WS2812
//#define MY_CONTROLLER	NEOPIXELS

#define MY_COLOR_ORDER	GRB
//#define MY_COLOR_ORDER	RGB

#define MY_COLOR_CORRECTION	TypicalPixelString
//#define MY_COLOR_CORRECTION	TypicalLEDStrip

#define STATUS_NONE				0
#define STATUS_WAITING			1
#define STATUS_PROCESSING		2
#define STATUS_CONFIGURE		3
#define STATUS_BOOTING			4

#define STATUS_ERROR_BASE		10
#define STATUS_ERROR_CONFIG		STATUS_ERROR_BASE+2
#define STATUS_ERROR_QUEUE		STATUS_ERROR_BASE+3
#define STATUS_ERROR_WIFI		STATUS_ERROR_BASE+4
#define STATUS_ERROR_DRIVER		STATUS_ERROR_BASE+5
#define STATUS_ERROR_GENERAL	STATUS_ERROR_BASE+6

#define STATUS_RESET			50

#ifdef __WITTY

#define BUILT_IN_LED		D4
#define GREEN_LED			D6
#define BLUE_LED			D7
#define RED_LED				D8
#define LDR					A0

#define LED_CONTROLLER_PIN	5


#else

#define BUILT_IN_LED		BUILTIN_LED
#define MY_LED_PIN			15

#endif


// What HW platform are we dealing with?
#ifdef __HOST_ESP8266
#define DEFAULT_LED_PIN 	D8
#endif
#ifdef __ARDUINO
#define DEFAULT_LED_PIN		3
#endif

// What LED string are we dealing with
#ifdef __LED_STRING
#define MY_CONTROLLER	WS2812
#endif

#ifdef __LED_STRIP
#define CONTROLLER	NEOPIXEL
#endif


#endif /* __CLIENT_GLOBAL_H_ */
