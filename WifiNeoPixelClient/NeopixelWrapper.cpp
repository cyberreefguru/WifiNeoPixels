/*
 * NeoPixelWrapper.cpp
 *
 *  Created on: Sep 12, 2015
 *      Author: tsasala
 */

#define FASTLED_ALLOW_INTERRUPTS 0
//#define __DEBUG

#include "NeopixelWrapper.h"


/**
 * Constructor
 */
NeopixelWrapper::NeopixelWrapper()
{
	leds = 0;
	maxIntensity = DEFAULT_INTENSITY;
//	intensity = DEFAULT_INTENSITY;
	gHue = 0;
//	sparkleCount = 0;
	maxFps = DEFAULT_FPS;
//	frameWaitTime = 1000/DEFAULT_FPS;
	gHueUpdateTime = 20;
}


/**
 * Initializes the library
 */
boolean NeopixelWrapper::initialize(uint8_t numLeds, uint8_t intensity)
{
	boolean status = false;

	leds = (CRGB *) malloc(sizeof(CRGB) * numLeds);
	if (leds != 0)
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
    	return NULL;
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

///**
// * Returns frames per second
// *
// */
//uint8_t NeopixelWrapper::getFramesPerSecond()
//{
//	return frameWaitTime*1000;
//}
//
///**
// * Changes the amount of times per second functions are
// * updated.
// */
//void NeopixelWrapper::setFramesPerSecond(uint8_t fps)
//{
//	frameWaitTime = 1000/fps;
//}

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
	if( FastLED.getBrightness() == 0)
	{
		FastLED.setBrightness( maxIntensity );
	}
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
	setPattern(0, FastLED.size(), pattern, 8, onColor, offColor, true);
}

/**
 * Rotates a pattern across the strip; onTime determines pause between rotation
 *
 * NOTE: Pattern repeats every 8 pixels
 */
void NeopixelWrapper::rotatePattern(uint16_t repeat, uint8_t pattern, uint8_t direction, CRGB onColor, CRGB offColor, uint32_t onTime, uint32_t offTime)
{
	uint16_t i, count;

	i = 0;
	count = 0;
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

		count++;
		if( repeat > 0 && count >= repeat )
		{
			break;
		}

	} // end while
}

/**
 * Turns on LEDs one at time in sequence.  LEFT = 0->n; RIGHT = n -> 0
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
void NeopixelWrapper::scrollPattern(uint8_t pattern, uint8_t direction, CRGB onColor, CRGB offColor, uint32_t onTime, uint32_t offTime, uint8_t clearAfter, uint8_t clearEnd)
{

	// TODO - implement pattern length

	uint8_t patternLength = 8;

	CRGB pixels[patternLength];
	int16_t curIndex;

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

//	// custom move window
//	// set off first
//	// set off when done if requested
//
//	fill(offColor, true);
//	if( direction == LEFT)
//	{
//		for(int16_t i=-7; i<FastLED.size(); i++)
//		{
//			setPatternTimed(i, pattern, onColor, offColor, onTime, offTime, clearAfter);
//			if(isCommandAvailable()) return;
//		}
//	}
//	else if (direction == RIGHT )
//	{
//		for(int16_t i=FastLED.size()-1; i>=-8; i--)
//		{
//			setPatternTimed(i, pattern, onColor, offColor, onTime, offTime, clearAfter);
//			if(isCommandAvailable()) return;
//		}
//	}

	if( clearEnd )
	{
		fill(offColor, true);
	}
}

/**
 *
 */
void NeopixelWrapper::bounce(uint16_t repeat, uint8_t pattern, uint8_t direction, CRGB onColor, CRGB offColor, uint32_t onTime, uint32_t offTime, uint32_t bounceTime, uint8_t clearAfter, uint8_t clearEnd)
{
	uint16_t count = 0;

	// custom bounce with 0-7, n-(n-7)

	while (isCommandAvailable() == false)
	{
		if( direction == LEFT )
		{
			scrollPattern(pattern, LEFT, onColor, offColor, onTime, offTime, clearAfter, clearEnd);
			if( commandDelay(bounceTime) ) return;
			scrollPattern(pattern, RIGHT, onColor, offColor, onTime, offTime, clearAfter, clearEnd);
			if( commandDelay(bounceTime) ) return;
		}
		else if (direction == RIGHT )
		{
			scrollPattern(pattern, RIGHT, onColor, offColor, onTime, offTime, clearAfter, clearEnd);
			if( commandDelay(bounceTime) ) return;
			scrollPattern(pattern, LEFT, onColor, offColor, onTime, offTime, clearAfter, clearEnd);
			if( commandDelay(bounceTime) ) return;
		}
		count++;
		if( repeat > 0 && count > repeat )
		{
			break;
		}
	} // end while

}


/**
 * Starts in the middle and works out; or starts in the end and works in
 */
void NeopixelWrapper::middle(uint16_t repeat, uint8_t direction, CRGB onColor, CRGB offColor, uint32_t onTime, uint32_t offTime, uint8_t clearAfter, uint8_t clearEnd)
{
	uint16_t count = 0;
	uint8_t numPixels = FastLED.size();
	uint8_t halfNumPixels = numPixels/2;

	if(clearEnd)
	{
		fill(offColor, true);
	}
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

		count++;
		if( repeat > 0 && count > repeat )
		{
			break;
		}

	} // end while
}

/**
 * Flashes random LED with specified color
 */
void NeopixelWrapper::randomFlash(uint32_t runTime, uint32_t onTime, uint32_t offTime, CRGB onColor, CRGB offColor)
{
	uint8_t i;

	fill(offColor, true);

	while (isCommandAvailable() == false)
	{
		i = random(FastLED.size());
		leds[i] = onColor;
		FastLED.show();
		if (commandDelay(onTime)) break;
		leds[i] = offColor;
		if (commandDelay(offTime)) break;
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
void NeopixelWrapper::strobe(uint32_t duration, CRGB onColor, CRGB offColor, uint32_t onTime, uint32_t offTime )
{
	FastLED.setBrightness(255);

	while( isCommandAvailable() == false )
	{
		fill(onColor, true);
		if( commandDelay(onTime) ) break;
		fill(offColor, true);
		if( commandDelay(offTime) ) break;
	}
}


/**
 * Creates lightning effort
 */
void NeopixelWrapper::lightning(CRGB onColor, CRGB offColor)
{

	uint32_t count, large;
	uint8_t i, b;

	b = false;
	FastLED.setBrightness(255);

	count = random(2, 6);
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
}

/**
 * Fills strip with rainbow pattern
 *
 * @glitter if true, randomly pops white into rainbow pattern
 */
void NeopixelWrapper::rainbow(uint32_t runTime, uint8_t glitterProbability, CRGB glitterColor, uint8_t fps)
{
	uint8_t frameTime = 1000/fps;

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
        EVERY_N_MILLISECONDS(gHueUpdateTime)
        {
            gHue++;
        } // slowly cycle the "base color" through the rainbow
    }

} // end rainbow

/**
 * This function draws rainbows with an ever-changing,widely-varying set of parameters.
 * https://gist.github.com/kriegsman/964de772d64c502760e5
 *
 */
void NeopixelWrapper::rainbowFade(uint32_t runTime, uint8_t fps)
{
	uint8_t frameTime = 1000/fps;

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
    }

} // end rainbow fade



/**
 * Creates random speckles of the specified color.
 *
 * 5-10 LEDs makes a nice effect
 *
 * NOTE: runTime has no effect at this time
 *
 */
void NeopixelWrapper::confetti(uint32_t runTime, CRGB color, uint8_t fadeBy, uint8_t fps)
{
	uint8_t frameTime = 1000/fps;

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

    }

} // end confetti


///**
// * Creates random speckles of the specified color.  Nearly the same as
// * confetti but fewer simulateous LEDs
// *
// */
//void NeoPixelWrapper::sparkle(CRGB color, uint8_t sparkles)
//{
//  while(isCommandAvailable() == false )
//  {
//      sparkleCount++;
//      fadeToBlackBy(leds, FastLED.size(), 5);
//      if( sparkleCount > sparkles )
//      {
//          int index = random16(FastLED.size());
//          while( leds[index] != (CRGB)CRGB::Black )
//          {
//              index = random16(FastLED.size());
//          }
//          leds[index] = color;
//          sparkleCount=0;
//      }
//      FastLED.show();
//      FastLED.delay( frameWaitTime );
//  }
//}



/**
 * Creates "cylon" pattern - bright led followed up dimming LEDs back and forth
 *
 */
void NeopixelWrapper::cylon(uint16_t repeat, CRGB color, uint32_t fadeTime, uint8_t fps)
{
	uint16_t count = 0;
	repeat = FastLED.size()*repeat;
//	uint8_t frameTime = 1000/fps;

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

		count++;
		if( repeat > 0 && count > repeat )
		{
			break;
		}

    }
} // end cylon

/**
 * No clue how to explain this one...
 *
 */
void NeopixelWrapper::bpm(uint32_t runTime, uint8_t fps)
{
	uint8_t frameTime = 1000/fps;
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
    }

}

/**
 * No clue how to explain this one
 *
 */
void NeopixelWrapper::juggle(uint32_t runTime, uint8_t fps)
{
	uint8_t frameTime = 1000/fps;
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
    }
}

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
    uint8_t index = startIndex;
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
    }

    if( show )
    {
    	FastLED.show();
    }

}

/**
 * Sets the color of the specified LED for onTime time.  If clearAfter
 * is true, returns color to original color and waits offTime before returning.
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
