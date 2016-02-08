/*
 * Helper.cpp
 *
 *  Created on: Oct 24, 2015
 *      Author: tsasala
 */

#include "Helper.h"

Helper::Helper()
{
	// TODO Auto-generated constructor stub

}

void Helper::dumpBuffer(uint8_t *buf, uint8_t len)
{
	printf("Buffer (0x%08xl) =", buf );
	for (uint8_t i = 0; i < len; i++)
	{
		printf("%.2X ", buf[i]);
	}
	printf("\n\r");
}

void Helper::error(uint8_t errorCode)
{
	pinMode(13, OUTPUT);
	while(1)
	{
		switch( errorCode )
		{
		case ERROR_WIRELESS:
			Helper::toggleLed(100);
			break;
		case ERROR_DRIVER:
			Helper::toggleLed(300);
			break;
		case ERROR_GENERAL:
			Helper::toggleLed(500);
			break;
		}
	}
}

void Helper::toggleLed(uint32_t time)
{
	digitalWrite(BUILTIN_LED, LOW);
	delay( time );
	digitalWrite(BUILTIN_LED, HIGH);
	delay( time );
}
