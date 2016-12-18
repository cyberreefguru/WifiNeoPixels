/*
 * Helper.h
 *
 *  Created on: Oct 24, 2015
 *      Author: tsasala
 */

#ifndef HELPER_H_
#define HELPER_H_

#include "Arduino.h"

#include "ClientGlobal.h"

#define ON		true
#define OFF		false

#define ERROR_CONFIG_TIME		100
#define ERROR_WIRELESS_TIME		200
#define ERROR_QUEUE_TIME		50
#define ERROR_DRIVER_TIME		25
#define ERROR_GENERAL_TIME		400

class Helper
{
public:
	Helper();
	static void workYield();
	static void delayWorker(uint32_t time);
	static void delayYield(uint32_t time);
	static void error();
	static void dumpBuffer(uint8_t *buf, uint8_t len);
	static void toggleLedTimed(uint32_t time);
	static void toggleLed();
	static void setLed(uint8_t b);
	static uint8_t getLed();

	static int8_t readChar(boolean b);
	static int16_t readInt(uint8_t *b, uint8_t len);
	static int16_t readString(uint8_t *b, uint8_t len);
	static void readAll();
};


//end of add your includes here
#ifdef __cplusplus
extern "C"
{
#endif

extern void worker();
extern uint8_t isCommandAvailable();
extern uint8_t commandDelay(uint32_t time);
extern void setStatus(volatile StatusEnum status);
extern volatile StatusEnum getStatus();
extern void yield();

//extern volatile uint8_t gStatus;

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HELPER_H_ */
