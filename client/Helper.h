/*
 * Helper.h
 *
 *  Created on: Oct 24, 2015
 *      Author: tsasala
 */

#ifndef HELPER_H_
#define HELPER_H_

#include "Arduino.h"

#define ERROR_WIRELESS			1
#define ERROR_DRIVER			2
#define ERROR_CONFIG			3
#define ERROR_GENERAL			20


class Helper
{
public:
	Helper();
	static void error(uint8_t error);
	static void dumpBuffer(uint8_t *buf, uint8_t len);
	static void toggleLed(uint32_t time);

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

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HELPER_H_ */
