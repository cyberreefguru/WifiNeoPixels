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

/**
 * Prints buffer to stdout as hex
 */
void Helper::dumpBuffer(uint8_t *buf, uint8_t len)
{
	printf("Buffer (0x%08xl) =", (uint32_t)buf );
	for (uint8_t i = 0; i < len; i++)
	{
		printf("%.2X ", buf[i]);
	}
	printf("\n\r");
}

/**
 * flashes LED based on error code received
 *
 */
void Helper::error(uint8_t errorCode)
{
	pinMode(13, OUTPUT);
	while(1)
	{
		switch( errorCode )
		{
		case ERROR_WIRELESS:
			Helper::toggleLed(300);
			break;
		case ERROR_DRIVER:
			Helper::toggleLed(200);
			break;
		case ERROR_GENERAL:
			Helper::toggleLed(100);
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

int8_t Helper::readChar(boolean echo)
{
	uint8_t flag = false;
	uint8_t c = 0;
	int16_t i = 0;

	// Wait for input
	while (!flag )
	{
		// wait until character is available
		while(!Serial.available() )
		{
			worker();
		}

		// read character
		c = Serial.read();

		if( echo )
		{
			// echo what the user typed
			Serial.print( (char)c );
		}

		// process received data
		switch( c )
		{
			case 0x03: // ^c - input aborted
				c = -1;
				flag = true;
				break;
			case '\n': // lf - input complete
				c = 0;
				flag = true;
				break;
			case'\r': // cr
				// do nothing - throw away
				break;
			default:
				flag = true;
				break;
		}

	}

	return c;
}

int16_t Helper::readInt(uint8_t *b, uint8_t len)
{
	int16_t i = -1;

	i = readString(b, len);
	if( i > 0 )
	{
		i = atoi((char *) b);
	}

#ifdef __DEBUG
			Serial.print(F("Int: "));
			Serial.println(i);
#endif

	return i;
}

/**
 * Reads a string from the serial port.  Reads until \n or \r encountered,
 * or len is hit.  -1 is returned if ^c is entered at any point.
 *
 */
int16_t Helper::readString(uint8_t *b, uint8_t len)
{

	uint8_t flag = false;
	uint8_t c = 0;
	int16_t i = 0;

	// Clear buffer - guarantees string is null terminated
	memset( b, 0, len );

	// Wait for input
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
			case 0x03: // ^c - input aborted
				memset( b, 0, len );
				i = -1;
				flag = true;
				break;
			case '\n': // lf - input complete
				flag = true;
				break;
			case'\r': // cr
				// do nothing - throw away
				break;
			default:
				b[i++] = c;
				break;
		}

		// check if we are at the end of the buffer
		if( i == (len-1) )
		{
			flag = true;
		}
	}

	// Read additional characters if any
	readAll();


#ifdef __DEBUG
	if( i > 0 )
	{
		Serial.print(F("String: "));
		Serial.println((char *) b);
	}
	else if( i == 0)
	{
		Serial.println(F("ERROR - no characters read"));
	}
	else if( i < 0 )
	{
		Serial.println(F("Input aborted"));
	}
#endif

	return i;

} // end readString


/**
 * Reads characters from input and discards
 *
 */
void Helper::readAll()
{
	uint8_t bytes;

	delay(100);// short delay in case more bytes come in

	// Read characters - discard since it just gets us into command mode
	while ( (bytes = Serial.available() ) )
	{
		for(uint i=0; i<bytes; i++)
		{
			Serial.read();
			worker();
		}
	}

}
