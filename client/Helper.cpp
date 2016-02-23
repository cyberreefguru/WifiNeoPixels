/*
 * Helper.cpp
 *
 *  Created on: Oct 24, 2015
 *      Author: tsasala
 */

#include "Helper.h"

Helper::Helper()
{
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


/**
 * Reads a string from the serial port.  Reads until \n or \r encountered.
 *
 */
uint8_t Helper::readString(uint8_t *b, uint8_t len)
{
	uint8_t flag = false;
	uint8_t c = 0;
	uint8_t i = 0;

	while (!flag )
	{
		// wait until character is available
		while(!Serial.available() )
		{
			worker();
		}

		// read character
		c = Serial.read();

		// echo what the user typed
		Serial.print( (char)c );

		// process received data
		switch( c )
		{
			case '\r':
				b[i++] = 0;
				flag = true;
				break;
			case'\n':
				// do nothing - throw away
				break;
			default:
				b[i++] = c;
				break;
		}

		// check if we are at the end of the buffer
		if( i == (len-1) )
		{
			b[i] = 0;
			flag = true;
		}
	}

	// Reading addition characters if they are there
	while( Serial.available() )
	{
		Serial.read();
	}

	return i;

}

