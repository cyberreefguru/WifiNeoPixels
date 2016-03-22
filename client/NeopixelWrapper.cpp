/*
 * NeoPixelWrapper.cpp
 *
 *  Created on: Sep 12, 2015
 *      Author: tsasala
 */

#define FASTLED_ALLOW_INTERRUPTS 0
//#define __DEBUG

#include "NeopixelWrapper.h"

void resetIntensity();


/**
 * Constructor
 */
NeopixelWrapper::NeopixelWrapper()
{
	leds = 0;
	gHue = 0;
	gHueUpdateTime = 20;
}


/**
 * Initializes the library
 */
boolean NeopixelWrapper::initialize(uint8_t numLeds, uint8_t intensity)
{
	boolean status = false;

	// Free memory if we already allocated it
	if( leds != 0 )
	{
		free(leds);
	}

	// Allocate memory for LED buffer
	leds = (CRGB *) malloc(sizeof(CRGB) * numLeds);
	if (leds == 0)
	{
		Serial.println(F("ERROR - unable to allocate LED memory"));
	}
	else
	{
		FastLED.addLeds<WS2812, D8>(leds, numLeds).setCorrection(TypicalLEDStrip);

		// set master brightness control
		FastLED.setBrightness(intensity);
		status = true;
	}

	return status;
}

/**
 * Returns color of pixel, or null if out of bounds
 *
 */
CRGB NeopixelWrapper::getPixel(int16_t index)
{
	if( index <0 || index >= FastLED.size() )
	{
	    return leds[index];
	}
	else
	{
#ifdef __DEBUG
    	Serial.print(F("WARN - pixel["));
    	Serial.print(index);
    	Serial.print(F("]="));
    	Serial.print(color, HEX);
    	Serial.println(F("-SKIPPING"));
#endif
    	return 0;
	}
}

/**
 * Sets the color of pixel.  No action if pixel is out of bounds
 *
 */
void NeopixelWrapper::setPixel(int16_t index, CRGB color, uint8_t show)
{
	if( index <0 || index >= FastLED.size() )
	{
	    leds[index] = color;
		if (show)
		{
			FastLED.show();
		}
	}
	else
	{
#ifdef __DEBUG
    	Serial.print(F("WARN - pixel["));
    	Serial.print(index);
    	Serial.print(F("]="));
    	Serial.print(color, HEX);
    	Serial.println(F("-SKIPPING"));
#endif
	}

}

void NeopixelWrapper::show()
{
	FastLED.show();
}


/**
 * Returns the hue update time
 */
uint8_t NeopixelWrapper::getHueUpdateTime()
{
	return gHueUpdateTime;
}

/**
 * Changes the amount of time to wait before updating the hue
 *
 */
void NeopixelWrapper::setHueUpdateTime(uint8_t updateTime)
{
	gHueUpdateTime = updateTime;
}

/**
 * Returns the hue update time
 */
uint8_t NeopixelWrapper::getIntensity()
{
	return FastLED.getBrightness();
}

/**
 * Changes the amount of time to wait before updating the hue
 *
 */
void NeopixelWrapper::setIntensity(uint8_t i)
{
	FastLED.setBrightness(i);
}

/**
 * fills all pixels with specified color
 *
 * @color - color to set
 * @show - if true, sets color immediately
 */
void NeopixelWrapper::fill(CRGB color, uint8_t show)
{
	resetIntensity();

	for (uint8_t i = 0; i < FastLED.size(); i++)
    {
        leds[i] = color;
    }
    if (show)
    {
        FastLED.show();
    }
}

/**
 * Fills pixels with specified pattern, starting at 0. 1 = on, 0 = off.
 *
 * Repeats pattern every 8 pixels.
 *
 */
void NeopixelWrapper::fillPattern(uint8_t pattern, CRGB onColor, CRGB offColor)
{
	resetIntensity();
	setPattern(0, FastLED.size(), pattern, 8, onColor, offColor, true);
}

/**
 * Rotates a pattern across the strip; onTime determines pause between rotation
 *
 * NOTE: Starts at 0, and repeats every 8 pixels through end of strip
 */
void NeopixelWrapper::rotatePattern(uint16_t repeat, uint32_t duration, uint8_t pattern, uint8_t direction, CRGB onColor, CRGB offColor, uint32_t onTime, uint32_t offTime)
{
	uint16_t i = 0;
	uint16_t count = 0;
	uint32_t endTime = millis() + duration;

	resetIntensity();

	while (isCommandAvailable() == false)
	{
		setPattern(0, FastLED.size(), pattern, 8, onColor, offColor, true);
		if (commandDelay(onTime)) break;
		if (direction == LEFT)
		{
			if (pattern & 0x80)
			{
				i = 0x01;
			}
			else
			{
				i = 0x00;
			}
			pattern = pattern << 1;
			pattern = pattern | i;
		}
		else if (direction == RIGHT)
		{
			if (pattern & 0x01)
			{
				i = 0x80;
			}
			else
			{
				i = 0x00;
			}
			pattern = pattern >> 1;
			pattern = pattern | i;
		}

		count += 1;
		if( repeat > 0 && count >= repeat )
		{
			break;
		}
		if( duration > 0 && millis() > endTime )
		{
			break;
		}

	} // end while
}

/**
 * Turns on LEDs one at time in sequence.  LEFT = 0->n; RIGHT = n -> 0
 *
 * NOTE: starts with pattern "off" the screen and scrolls "on" the screen,
 *       then "off" the screen again
 *
 * @pattern - the pattern to wipe
 * @direction - left (up) or right (down)
 * @onColor - color to fill LEDs with
 * @offColor - color to fill LEDs with
 * @onTime - time to keep LED on
 * @offTime - time to keep LED off
 * @clearAfter - turn LED off after waiting
 * @clearEnd - clear after complete
 */
void NeopixelWrapper::scrollPattern(uint16_t repeat, uint32_t duration, uint8_t pattern, uint8_t patternLength, uint8_t direction, CRGB onColor, CRGB offColor, uint32_t onTime, uint32_t offTime, uint8_t clearAfter, uint8_t clearEnd)
{
	uint16_t count = 0;
	uint32_t endTime = millis() + duration;

	CRGB pixels[patternLength];
	int16_t curIndex;

	resetIntensity();

	// Initialize the pixel buffer
	for(uint8_t i=0; i<patternLength; i++)
	{
			// rotates pattern and tests for "on"
			if ((pattern >> i) & 0x01)
			{
				pixels[i] = onColor;
			}
			else
			{
				pixels[i] = offColor;
			}
	}

	// clear LEDs
	fill(offColor, true);


	while (isCommandAvailable() == false)
	{
		// Loop through all LEDs (plus some)
		for(int16_t j=0; j<(FastLED.size()+patternLength-1); j++ )
		{
			// Set start location
			if( direction == LEFT )
			{
				if( (j >= (patternLength-1)) && (j <= (FastLED.size() - 1)) )
				{
					// copy all pixels to leds
					curIndex = j;
					for(uint8_t i=0; i<patternLength; i++)
					{
						leds[curIndex--] = pixels[i];
					}
				}
				else if( j <= (patternLength-1) )
				{
					curIndex = j;
					uint8_t bitsToCopy = j+1;

					for(uint8_t i=0; i< bitsToCopy; i++)
					{
						leds[curIndex--] = pixels[i];
					}
				}
				else if(j > (FastLED.size()-1))
				{
					curIndex = FastLED.size()-1;
					uint8_t bitsToCopy = (patternLength-1) - (j-FastLED.size());
					uint8_t start = (patternLength-bitsToCopy);
	#ifdef __DEBUG
					Serial.print(F("j="));
					Serial.print(j);
					Serial.print(F(", curIndex="));
					Serial.print(curIndex);
					Serial.print(F(", bitsToCopy="));
					Serial.print(bitsToCopy);
					Serial.print(F(", start="));
					Serial.println(start);
	#endif
					for(uint8_t i=start; i< 8; i++)
					{
	#ifdef __DEBUG
						Serial.print(F("curIndex="));
						Serial.print(curIndex);
						Serial.print(F(", i="));
						Serial.println(i);
	#endif
						leds[curIndex--] = pixels[i];
					}
				}
			} // end if LEFT
			else if(direction == RIGHT )
			{
				if( (j >= (patternLength-1)) && (j <= (FastLED.size() - 1)) )
				{
					// copy all pixels to leds
					curIndex = FastLED.size() - j - 1;
					for(uint8_t i=0; i<patternLength; i++)
					{
						leds[curIndex++] = pixels[i];
					}
				}
				else if( j < (patternLength-1) )
				{
					curIndex = FastLED.size() - j - 1;
					uint8_t bitsToCopy = FastLED.size() - curIndex;

					for(uint8_t i=0; i< bitsToCopy; i++)
					{
						leds[curIndex++] = pixels[i];
					}
				}
				else if(j > (FastLED.size()-1))
				{
					curIndex = 0;
					uint8_t bitsToCopy = (patternLength-1) - (j-FastLED.size());
					uint8_t start = (patternLength-bitsToCopy);
	#ifdef __DEBUG
					Serial.print(F("j="));
					Serial.print(j);
					Serial.print(F(", curIndex="));
					Serial.print(curIndex);
					Serial.print(F(", bitsToCopy="));
					Serial.print(bitsToCopy);
					Serial.print(F(", start="));
					Serial.println(start);
	#endif
					for(uint8_t i=start; i< 8; i++)
					{
	#ifdef __DEBUG
						Serial.print(F("curIndex="));
						Serial.print(curIndex);
						Serial.print(F(", i="));
						Serial.println(i);
	#endif
						leds[curIndex++] = pixels[i];
					}
				}
			} //end if RIGHT

			FastLED.show();
			commandDelay(onTime);
			if( clearAfter )
			{
				fill(offColor, false);
			}

		} // end for j

		count += 1;
		if( repeat > 0 && count >= repeat )
		{
			break;
		}
		if( duration > 0 && millis() > endTime )
		{
			break;
		}

	} // end while

	if( clearEnd )
	{
		fill(offColor, true);
	}
}

/**
 * Bounces the specified pattern the specified direction.
 *
 * NOTE: Pattern does not double flash at the ends.
 *
 */
void NeopixelWrapper::bounce(uint16_t repeat, uint32_t duration, uint8_t pattern, uint8_t patternLength, uint8_t direction, CRGB onColor, CRGB offColor, uint32_t onTime, uint32_t offTime, uint32_t bounceTime, uint8_t clearAfter, uint8_t clearEnd)
{
	// custom bounce with 0-7, n-(n-7)
	resetIntensity();

	uint16_t count = 0;
	uint32_t endTime = millis() + duration;

	uint8_t first = true;
	uint8_t lstart = 0;
	uint8_t lend = 0;
	uint8_t rstart = 0;
	uint8_t rend = 0;

	resetIntensity();

	if (direction == LEFT)
	{
		lstart = 0;
		lend = FastLED.size() - patternLength;

		rstart = FastLED.size() - patternLength - 1;
		rend = 0;
	}
	else if (direction == RIGHT)
	{
		rstart = FastLED.size() - patternLength;
		rend = 0;

		lstart = 1;
		lend = FastLED.size() - patternLength;
	}
	else
	{
		return;
	}

	// Fill with off color
	fill(offColor, true);

	// work until we have a new command
	while (isCommandAvailable() == false)
	{
		if( direction == LEFT )
		{
			for(int16_t i=lstart; i<=lend; i++ )
			{
				setPattern(i, patternLength, pattern, patternLength, onColor, offColor, true);
				if (commandDelay(onTime)) break;
				if( clearAfter )
				{
					fill(offColor, true);
					if (commandDelay(offTime)) break;
				}
			}
			if( clearEnd )
			{
				fill(offColor, true);
				if (commandDelay(bounceTime)) break;
			}

			for(int16_t i=rstart; i>=rend; i-=1 )
			{
				setPattern(i, patternLength, pattern, patternLength, onColor, offColor, true);
				if (commandDelay(onTime)) break;
				if( clearAfter )
				{
					fill(offColor, true);
					if (commandDelay(offTime)) break;
				}
			}
			if( clearEnd )
			{
				fill(offColor, true);
				if (commandDelay(bounceTime)) break;
			}

			if( first )
			{
				lstart = 1;
				lend = FastLED.size() - patternLength - 1;
				rstart = FastLED.size() - patternLength;
				rend = 0;
				first = false;
			}
		}
		else if(direction == RIGHT )
		{
			for(int16_t i=rstart; i>=rend; i-=1 )
			{
				setPattern(i, patternLength, pattern, patternLength, onColor, offColor, true);
				if (commandDelay(onTime)) break;
				if( clearAfter )
				{
					fill(offColor, true);
					if (commandDelay(offTime)) break;
				}
			}
			if( clearEnd )
			{
				fill(offColor, true);
				if (commandDelay(bounceTime)) break;
			}
			for(int16_t i=lstart; i<=lend; i++ )
			{
				setPattern(i, patternLength, pattern, patternLength, onColor, offColor, true);
				if (commandDelay(onTime)) break;
				if( clearAfter )
				{
					fill(offColor, true);
					if (commandDelay(offTime)) break;
				}
			}
			if( clearEnd )
			{
				fill(offColor, true);
				if (commandDelay(bounceTime)) break;
			}

			if( first )
			{
				rstart = FastLED.size() - patternLength -1;
				rend = 0;

				lstart = 1;
				lend = FastLED.size() - patternLength;
				first = false;
			}

		}

		count += 1;
		if( repeat > 0 && count >= repeat )
		{
			break;
		}
		if( duration > 0 && millis() > endTime )
		{
			break;
		}

	} // end while

} // end bounce


/**
 * Starts in the middle and works out; or starts in the end and works in
 */
void NeopixelWrapper::middle(uint16_t repeat, uint32_t duration, uint8_t direction, CRGB onColor, CRGB offColor, uint32_t onTime, uint32_t offTime, uint8_t clearAfter, uint8_t clearEnd)
{
	uint16_t count = 0;
	uint32_t endTime = millis() + duration;;

	uint8_t numPixels = FastLED.size();
	uint8_t halfNumPixels = numPixels/2;

	resetIntensity();
	fill(offColor, true);

	while (isCommandAvailable() == false)
	{
		if(direction == IN)
		{
			for(uint8_t i=0; i<halfNumPixels; i++)
			{
				leds[i] = onColor;
				leds[(numPixels-1)-i] = onColor;
				FastLED.show();
				if( commandDelay(onTime) ) return;

				if( clearAfter == true )
				{
					leds[i] = offColor;
					leds[(numPixels-1)-i] = offColor;
					FastLED.show();
					if( commandDelay(offTime) ) return;
				}
			}
		}
		else if( direction == OUT )
		{
			for(uint8_t i=0; i<halfNumPixels+1; i++)
			{
				leds[halfNumPixels-i] = onColor;
				leds[halfNumPixels+i] = onColor;
				FastLED.show();
				if( commandDelay(onTime) ) return;

				if( clearAfter == true )
				{
					leds[halfNumPixels-i] = offColor;
					leds[halfNumPixels+i] = offColor;
					FastLED.show();
					if( commandDelay(offTime) ) return;
				}
			}
		}
		if(clearEnd)
		{
			fill(offColor, true);
		}

		count += 1;
		if( repeat > 0 && count >= repeat )
		{
			break;
		}

		if( duration > 0 && millis() > endTime )
		{
			break;
		}

	} // end while
}

/**
 * Flashes random LED with specified color
 */
void NeopixelWrapper::randomFlash(uint16_t repeat, uint32_t duration, uint32_t onTime, uint32_t offTime, CRGB onColor, CRGB offColor)
{
	uint16_t count = 0;
	uint32_t endTime = millis() + duration;;

	uint8_t i;

	resetIntensity();
	fill(offColor, true);

	while (isCommandAvailable() == false)
	{
		i = random(FastLED.size());
		leds[i] = onColor;
		FastLED.show();
		if (commandDelay(onTime)) break;
		leds[i] = offColor;
		if (commandDelay(offTime)) break;

		count += 1;
		if( repeat > 0 && count >= repeat )
		{
			break;
		}

		if( duration > 0 && millis() > endTime )
		{
			break;
		}

	}

	fill(offColor, true);

} // randomFlash


/**
 * Fades LEDs up or down with the specified time increment
 */
void NeopixelWrapper::fade(uint8_t direction, uint8_t fadeIncrement, uint32_t time, CRGB color)
{
	int16_t i=0;

	if( direction == DOWN )
	{
		FastLED.setBrightness(255);
		FastLED.showColor(color);
	}
	else if( direction == UP )
	{
		FastLED.setBrightness(0);
		FastLED.showColor(color);
	}
	while(i<255)
	{
		if( commandDelay(time) ) break;
		i = i+fadeIncrement;

		if( i > 255)
		{
			i = 255;
		}

		if( direction == DOWN )
		{
			FastLED.setBrightness(255-i);
		}
		else if( direction == UP)
		{
			FastLED.setBrightness(i);
		}
		FastLED.showColor(color);

	}

}

/**
 * Flashes LEDs
 */
void NeopixelWrapper::strobe(uint16_t repeat, uint32_t duration, CRGB onColor, CRGB offColor, uint32_t onTime, uint32_t offTime )
{
	uint16_t count = 0;
	uint32_t endTime = millis() + duration;;

	resetIntensity();

	while( isCommandAvailable() == false )
	{
		fill(onColor, true);
		if( commandDelay(onTime) ) break;
		fill(offColor, true);
		if( commandDelay(offTime) ) break;

		count += 1;
		if( repeat > 0 && count >= repeat )
		{
			break;
		}

		if( duration > 0 && millis() > endTime )
		{
			break;
		}

	} // end while command
}


/**
 * Creates lightning effort
 */
void NeopixelWrapper::lightning(uint16_t repeat, uint32_t duration, CRGB onColor, CRGB offColor)
{
	uint16_t count = 0;
	uint32_t endTime = millis() + duration;;

	uint32_t lc, large;
	uint8_t i, b;

	b = false;
	resetIntensity();

    while(isCommandAvailable() == false )
	{
		lc = random(2, 6);
		for(i=0; i<count; i++)
		{
			large = random(0,100);
			fill(onColor, true);
			if( large > 40 && b == false)
			{
				if( commandDelay(random(100, 350)) ) break;
				b = true;
			}
			else
			{
				if( commandDelay(random(20, 50)) ) break;
			}
			fill(offColor, true);
			if( large > 40 && b == false )
			{
				if( commandDelay(random(200, 500)) ) break;
			}
			else
			{
				if( commandDelay(random(30, 70)) ) break;
			}
		}

		count += 1;
		if( repeat > 0 && count >= repeat )
		{
			break;
		}

		if( duration > 0 && millis() > endTime )
		{
			break;
		}

	} // end while command
}

/**
 * Fills strip with rainbow pattern
 *
 * @glitter if true, randomly pops white into rainbow pattern
 */
void NeopixelWrapper::rainbow(uint32_t duration, uint8_t glitterProbability, CRGB glitterColor, uint8_t fps)
{
	uint32_t endTime = millis() + duration;;

	//TODO: Figure out to better control timing with FPS or hue update time

	uint8_t frameTime = 1000/fps;
	resetIntensity();

    while(isCommandAvailable() == false )
    {
        // FastLED's built-in rainbow generator
        fill_rainbow(leds, FastLED.size(), gHue, 7);
        if (glitterProbability > 0)
        {
            if (random8() < glitterProbability)
            {
                leds[random16(FastLED.size())] += glitterColor;
            }

        }
        FastLED.show();
        commandDelay( frameTime );

        // do some periodic updates
        // TODO: EVERY_N_MILLISECONDS does not appear to work on the ESP8266
        EVERY_N_MILLISECONDS(gHueUpdateTime)
        {
            gHue++;
        } // slowly cycle the "base color" through the rainbow

		if( duration > 0 && millis() > endTime )
		{
			break;
		}

    } // end while command

} // end rainbow

/**
 * This function draws rainbows with an ever-changing,widely-varying set of parameters.
 * https://gist.github.com/kriegsman/964de772d64c502760e5
 *
 */
void NeopixelWrapper::rainbowFade(uint32_t duration, uint8_t fps)
{
	uint32_t endTime = millis() + duration;;

	//TODO: Figure out to better control timing with FPS or hue update time

	uint8_t frameTime = 1000/fps;
	resetIntensity();

    while(isCommandAvailable() == false )
    {

        static uint16_t sPseudotime = 0;
        static uint16_t sLastMillis = 0;
        static uint16_t sHue16 = 0;

        uint8_t sat8 = beatsin88(87, 220, 250);
        uint8_t brightdepth = beatsin88(341, 96, 224);
        uint16_t brightnessthetainc16 = beatsin88(203, (25 * 256), (40 * 256));
        uint8_t msmultiplier = beatsin88(147, 23, 60);

        uint16_t hue16 = sHue16; //gHue * 256;
        uint16_t hueinc16 = beatsin88(113, 1, 3000);

        uint16_t ms = millis();
        uint16_t deltams = ms - sLastMillis;
        sLastMillis = ms;
        sPseudotime += deltams * msmultiplier;
        sHue16 += deltams * beatsin88(400, 5, 9);
        uint16_t brightnesstheta16 = sPseudotime;

        for (uint16_t i = 0; i < (uint16_t) FastLED.size(); i++)
        {
            hue16 += hueinc16;
            uint8_t hue8 = hue16 / 256;

            brightnesstheta16 += brightnessthetainc16;
            uint16_t b16 = sin16(brightnesstheta16) + 32768;

            uint16_t bri16 = (uint32_t) ((uint32_t) b16 * (uint32_t) b16) / 65536;
            uint8_t bri8 = (uint32_t) (((uint32_t) bri16) * brightdepth) / 65536;
            bri8 += (255 - brightdepth);

            CRGB newcolor = CHSV(hue8, sat8, bri8);

            uint16_t pixelnumber = i;
            pixelnumber = (FastLED.size() - 1) - pixelnumber;

            nblend(leds[pixelnumber], newcolor, 64);
        }

        FastLED.show();
        commandDelay( frameTime );

		if( duration > 0 && millis() > endTime )
		{
			break;
		}

    } // end while command

} // end rainbow fade



/**
 * Creates random speckles of the specified color.
 *
 * 5-10 LEDs makes a nice effect
 *
 * NOTE: runTime has no effect at this time
 *
 */
void NeopixelWrapper::confetti(uint32_t duration, CRGB color, uint8_t fadeBy, uint8_t fps)
{
	uint32_t endTime = millis() + duration;;

	//TODO: Figure out to better control timing with FPS or hue update time

	uint8_t frameTime = 1000/fps;
	resetIntensity();

	while(isCommandAvailable() == false )
    {
        // random colored speckles that blink in and fade smoothly
        fadeToBlackBy(leds, FastLED.size(), fadeBy);
        int pos = random16(FastLED.size());
        if (color == (CRGB)RAINBOW)
        {
            leds[pos] += CHSV(gHue + random8(64), 200, 255);
            // do some periodic updates
            EVERY_N_MILLISECONDS(gHueUpdateTime)
            {
                gHue++;
            } // cycle the "base color" through the rainbow

        }
        else
        {
            leds[pos] += color;
        }
        FastLED.show();
        commandDelay( frameTime );

		if( duration > 0 && millis() > endTime )
		{
			break;
		}

    } // end while command

} // end confetti


/**
 * Creates "cylon" pattern - bright led followed up dimming LEDs back and forth
 *
 */
void NeopixelWrapper::cylon(uint16_t repeat, uint32_t duration, CRGB color, uint32_t fadeTime, uint8_t fps)
{
	uint16_t count = 0;
	uint32_t endTime = millis() + duration;;
	uint8_t flag = false; // flag to increment counter

	//TODO: Figure out to better control timing with FPS or hue update time

	resetIntensity();

    while(isCommandAvailable() == false )
    {
        fadeToBlackBy(leds, FastLED.size(), 20);
        int pos = beatsin16(fps, 0, FastLED.size());
        if (color == (CRGB)RAINBOW)
        {
            leds[pos] += CHSV(gHue, 255, 192);
            // do some periodic updates
            EVERY_N_MILLISECONDS(gHueUpdateTime)
            {
                // slowly cycle the "base color" through the rainbow
                gHue++;
            }
        }
        else
        {
            leds[pos] += color;
        }

        FastLED.show();
        commandDelay( fadeTime );

        if( pos == 0 && flag == false)
        {
			count += 1;
			flag = true; // set flag
        }
        if( flag == true && pos != 0 )
        {
        	flag = false; // wait for beats to go past 0
        }
		if( repeat > 0 && count >= repeat )
		{
			break;
		}
		if( duration > 0 && millis() > endTime )
		{
			break;
		}

    } // end while command

} // end cylon

/**
 * No clue how to explain this one...
 *
 */
void NeopixelWrapper::bpm(uint32_t duration, uint8_t fps)
{
	uint32_t endTime = millis() + duration;;

	//TODO: Figure out to better control timing with FPS or hue update time
	uint8_t frameTime = 1000/fps;
	resetIntensity();

	while(isCommandAvailable() == false )
    {
        // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
        uint8_t BeatsPerMinute = 62;
        CRGBPalette16 palette = PartyColors_p;
        uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
        for (int i = 0; i < FastLED.size(); i++)
        { //9948
            leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
        }
        // do some periodic updates
        EVERY_N_MILLISECONDS(gHueUpdateTime)
        {
            gHue++;
        } // slowly cycle the "base color" through the rainbow

        FastLED.show();
        commandDelay( frameTime );

		if( duration > 0 && millis() > endTime )
		{
			break;
		}

    }

}

/**
 * No clue how to explain this one
 *
 */
void NeopixelWrapper::juggle(uint32_t duration, uint8_t fps)
{
	uint32_t endTime = millis() + duration;;

	//TODO: Figure out to better control timing with FPS or hue update time
	uint8_t frameTime = 1000/fps;
	resetIntensity();

	while(isCommandAvailable() == false )
    {
        // eight colored dots, weaving in and out of sync with each other
        fadeToBlackBy(leds, FastLED.size(), 20);
        byte dothue = 0;
        for (int i = 0; i < 8; i++)
        {
            leds[beatsin16(i + 7, 0, FastLED.size())] |= CHSV(dothue, 200, 255);
            dothue += 32;
        }

        FastLED.show();
        commandDelay( frameTime );

		if( duration > 0 && millis() > endTime )
		{
			break;
		}

    }
}

/**
 * Stacks LEDs based on direction
 *
 */
void NeopixelWrapper::stack(uint16_t repeat, uint32_t duration, uint8_t direction, CRGB onColor, CRGB offColor, uint32_t onTime, uint8_t clearEnd)
{
	uint8_t index;
	uint16_t count;
	uint32_t endTime;

	resetIntensity();
	fill(offColor, true);

	count = 0;
	endTime = millis() + duration;

	while(isCommandAvailable() == false )
    {

		if( direction == DOWN )
		{
			index = 0;
		}
		else
		{
			index = FastLED.size()-1;
		}

		for(uint8_t i=0; i<FastLED.size(); i++ )
		{
			if(direction == DOWN )
			{
				for(int16_t j=FastLED.size(); j>index; j -= 1 )
				{
					if( onColor == (CRGB)RAINBOW )
					{
						leds[j] = CHSV(random8(0, 255), 255, 255);
					}
					else
					{
						leds[j] = onColor;
					}
					FastLED.show();
					if( commandDelay( onTime) ) return;
					leds[j] = offColor;
					FastLED.show();
				}
				if( onColor == (CRGB)RAINBOW )
				{
					leds[index] = CHSV(random8(0, 255), 255, 255);
				}
				else
				{
					leds[index] = onColor;
				}
				FastLED.show();
				index += 1;
			}
			else if( direction == UP )
			{
				for(int16_t j=0; j<index; j += 1 )
				{
					if( onColor == (CRGB)RAINBOW )
					{
						leds[j] = CHSV(random8(0, 255), 255, 255);
					}
					else
					{
						leds[j] = onColor;
					}
					FastLED.show();
					if( commandDelay( onTime) ) return;
					leds[j] = offColor;
					FastLED.show();
				}
				if( onColor == (CRGB)RAINBOW )
				{
					leds[index] = CHSV(random8(0, 255), 255, 255);
				}
				else
				{
					leds[index] = onColor;
				}
				FastLED.show();
				index -= 1;
			}
		}

		if( clearEnd == true )
		{
			fill(offColor, true);
		}

		count += 1;
		if( repeat > 0 && count >= repeat )
		{
			break;
		}

		if( duration > 0 && millis() > endTime )
		{
			break;
		}


	} // end while command == false

} // end stack

/**
 * Randomly fills string with specified color
 *
 */
void NeopixelWrapper::fillRandom(uint16_t repeat, uint32_t duration, CRGB onColor, CRGB offColor, uint32_t onTime, uint32_t offTime, uint8_t clearAfter, uint8_t clearEnd)
{
	uint8_t total;
	uint8_t index;
	uint16_t count;
	uint32_t endTime;
	uint8_t flag = false;

	resetIntensity();
	fill(offColor, true);

	count = 0;
	endTime = millis() + duration;

	CRGB color = onColor;

	while(isCommandAvailable() == false )
    {
		total = FastLED.size();
		while( total > 0 )
		{
			index = random8(0, FastLED.size());
			if( onColor == (CRGB)RAINBOW )
			{
				color = CHSV(random8(0, 255), 255, 255);
			}
			if( leds[index] == offColor || flag == true )
			{
				leds[index] = color;
				FastLED.show();
				total--;
				if( commandDelay(onTime)) return;
				if( clearAfter )
				{
					leds[index] = offColor;
					if( commandDelay(offTime)) return;
				}
			}
		}
		flag = true;
		if( clearEnd )
		{
			fill(offColor, true);
			flag = false;
		}

		count += 1;
		if( repeat > 0 && count >= repeat )
		{
			break;
		}

		if( duration > 0 && millis() > endTime )
		{
			break;
		}

	} // while command == false


} // end randomFill


////////////////////////////////////////
// BEGIN PRIVATE FUNCTIONS
////////////////////////////////////////
/**
 * Sets the color of the specified LED for onTime time.  If clearAfter
 * is true, returns color to original color and waits offTime before returning.
 */
void NeopixelWrapper::setPatternTimed(int16_t startIndex, uint8_t pattern, CRGB onColor, CRGB offColor, uint32_t onTime, uint32_t offTime, uint8_t clearAfter)
{
	CRGB currentColor[8];

	for(uint8_t i=0; i<8; i++)
	{
		if(startIndex+i < FastLED.size() && startIndex+i >= 0)
		{
			currentColor[i] = leds[startIndex+i];
		}
	}
	setPattern(startIndex, 8, pattern, 8, onColor, offColor, true);
    commandDelay(onTime);

    if(clearAfter == true)
    {
    	for(uint8_t i=0; i<8; i++)
    	{
    		if(startIndex+i < FastLED.size() && startIndex+i >= 0)
    		{
        		leds[startIndex+i] = currentColor[i];
    		}
    	}
        FastLED.show();
        commandDelay(offTime);
    }
}


/**
 * Sets the color of the specified LED for onTime time.  If clearAfter
 * is true, returns color to original color and waits offTime before returning.
 */
void NeopixelWrapper::setPixelTimed(int16_t index, CRGB newColor, uint32_t onTime, uint32_t offTime, uint8_t clearAfter)
{
    CRGB curColor;

    curColor = leds[index];
    leds[index] = newColor;
    FastLED.show();
    commandDelay(onTime);
    if(clearAfter == true)
    {
        leds[index] = curColor;
        FastLED.show();
        commandDelay(offTime);
    }

}


/**
 * Fills the pixels with the specified pattern, starting at the specified index.
 * Stops filling when length is hit.
 *
 */
void NeopixelWrapper::setPattern(int16_t startIndex, uint8_t length, uint8_t pattern, uint8_t patternLength, CRGB onColor, CRGB offColor, uint8_t show)
{
    int16_t index;
    uint8_t patternIndex = 0;

#ifdef __DEBUG
    	Serial.print(F("setPattern(start="));
    	Serial.print(startIndex);
    	Serial.print(F(", len="));
    	Serial.print(length);
    	Serial.print(F(", pattern="));
    	Serial.print(pattern);
    	Serial.print(F(", pl="));
    	Serial.print(patternLength);
    	Serial.println(F(")"));
#endif

    for(index=0; index<length; index++)
    {
    	// Safety measure - allows pattern length to be > amount of pixels left
		if( (startIndex+index) >= FastLED.size())
		{
#ifdef __DEBUG
    	Serial.print(F("WARN - pixel["));
    	Serial.print(startIndex+index);
    	Serial.print(F("]="));
    	Serial.print(onColor, HEX);
    	Serial.println(F("-SKIPPING"));
#endif
		}
		else if( (startIndex+index) < 0)
		{
#ifdef __DEBUG
    	Serial.print(F("WARN - pixel["));
    	Serial.print(startIndex+index);
    	Serial.print(F("]="));
    	Serial.print(onColor, HEX);
    	Serial.println(F("-SKIPPING"));
#endif
		}
		else
		{
			// rotates pattern and tests for "on"
			if ((pattern >> patternIndex) & 0x01)
			{
				leds[startIndex+index] = onColor;
#ifdef __DEBUG
			Serial.print(F("INFO - pixel["));
			Serial.print(startIndex+index);
			Serial.print(F("]="));
			Serial.println(onColor, HEX);
#endif
			}
			else
			{
				leds[startIndex+index] = offColor;
#ifdef __DEBUG
			Serial.print(F("INFO - pixel["));
			Serial.print(startIndex+index);
			Serial.print(F("]="));
			Serial.println(offColor, HEX);
#endif
			}
		}

        // increases pattern index
        patternIndex += 1;
        // test if we need to reset pattern index
        if( patternIndex == patternLength)
        {
        	patternIndex = 0;
        }

    } // end for

    if( show )
    {
    	FastLED.show();
    }

}

/**
 * Sets the color of the specified LED.
 */
void NeopixelWrapper::setPixel(int16_t index, CRGB color)
{
	if( index <0 || index >= FastLED.size() )
	{
	    leds[index] = color;
	}
	else
	{
#ifdef __DEBUG
    	Serial.print(F("WARN - pixel["));
    	Serial.print(index);
    	Serial.print(F("]="));
    	Serial.print(color, HEX);
    	Serial.println(F("-SKIPPING"));
#endif
	}
}

/**
 * If current intensity is 0, reset to default
 *
 */
void resetIntensity()
{
	if( FastLED.getBrightness() == 0)
	{
		FastLED.setBrightness( DEFAULT_INTENSITY );
	}
}
