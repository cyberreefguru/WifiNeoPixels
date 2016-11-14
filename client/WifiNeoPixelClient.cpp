/*
 *  Client for controlling LEDs with Node MCU
 *
 */

#include "WifiNeoPixelClient.h"

// Internal Variables
static Configuration config;
static NeopixelWrapper controller;
static PubSubWrapper pubsubw;
static WifiWrapper wifiw;
static Menu menu;

// software timer instance
os_timer_t ledTimer;
volatile uint8_t gLedCounter;
volatile uint8_t gLedState;

// Indicates we have a command waiting for processing
static volatile uint8_t commandAvailable = false;

// Internal functions
void configure();
void parseCommand();
boolean initialize();
void ledTimerCallback(void *pArg);


/**
 * Setup function
 *
 */
void setup()
{
	// Configure serial port
	Serial.begin(115200);
	Serial.println(F("\n\rLED Controller Powered Up"));

	// Basic HW setup

	// This pauses the start process so the user can do things like
	// attach to the serial port and look at debug information :)
	pinMode(BUILT_IN_LED, OUTPUT);     // Initialize the  pin as an output
	digitalWrite(BUILT_IN_LED, LOW);
	delay(2000);
	yield();
	digitalWrite(BUILT_IN_LED, HIGH);
	delay(2000);
	yield();
	digitalWrite(BUILT_IN_LED, LOW);
	delay(2000);
	yield();

	// Initialize the configuration object; configs stored in Flash
	Serial.println("\nInitializing configuration...");
    if( config.initialize() )
    {
        Serial.println("\nConfiguration initialized.");
    }
    else
    {
    	Serial.println("\n**ERROR - failed to initialize configuration");
    	Helper::error(ERROR_CONFIG); // never returns from here
    }

    // Initialize menu
	Serial.println("\nInitializing menu...");
	menu.initialize(&config);

	// Initialize wifi, pubsub, and LEDs
	if( !initialize() )
	{
		menu.waitForConfig();
		configure();
	}

    Serial.println(F("** Initialization Complete **"));

    // Turn off LED
	digitalWrite(BUILT_IN_LED, HIGH);

	// Set LED variables
	gLedCounter = 0;
	gLedState = STATE_LED_WAITING;

	// Setup and initiate internal timer
	os_timer_setfn(&ledTimer, ledTimerCallback, NULL);
	os_timer_arm(&ledTimer, 125, true);
}

/**
 * Loop function
 *
 */
void loop()
{
	uint8_t configFlag = 0;

	// calls yield and other must run code
	worker();

	// Check if command is available
	if( isCommandAvailable() )
	{
		parseCommand();
	}

	// check if we are connected to mqtt server
	if( !pubsubw.checkConnection() )
	{
		// If we lost the connection to the queue toggle the LED as a warning to the user
		// We can't do anything other than try to connect or have the user reconfigure
		gLedState = STATE_LED_ERROR;
		Helper::toggleLed(ERROR_QUEUE_TIME);
	}
	else
	{
		gLedState = STATE_LED_WAITING;
	}

	// Check if user wants to configure node
	if( Serial.available() )
	{
		configFlag = 1;
	}

	// Check if we need to enter command mode to reconfigure the node
	if( configFlag == 1)
	{
		configFlag = 0;
		configure();
	}

} // end loop

/**
 * Enters configure mode, then waits until power is cycled
 */
void configure()
{
	if( menu.configure() == 1 )
	{
		// NOTE: I could not get the device reboot reliably, so cycle the power
		//       is the best option.  Even "reset" and "restart" don't work reliably.

		Serial.println(F("\nUser Configuration Complete - please cycle power."));
		gLedState = STATE_LED_ERROR;
		Helper::error(ERROR_GENERAL);
	}
	else
	{
		Serial.println("Configuration aborted!");
	}

}


/**
 * Initializes the node.  Returns true if node configured properly, false otherwise.
 *
 */
boolean initialize()
{
	boolean configured = false;

	Serial.println(F("Configuring wifi..."));

	// Initialize WIFI
	if( wifiw.initialize(&config) )
	{
		Serial.println(F("Configuring queue..."));

		// Initialize pubsub
		if( pubsubw.initialize(&config, &wifiw) )
		{
			Serial.println(F("Configuring leds..."));

			// Initialize the LEDs
			if ( controller.initialize(config.getNumberLeds(), DEFAULT_INTENSITY) )
			{
				Serial.print(F("\nLED Controller initialized..."));
				controller.fill(CRGB::White, true);
				delay(250);
				controller.fill(CRGB::Black, true);
				delay(250);
				controller.fill(CRGB::Red, true);
				delay(250);
				controller.fill(CRGB::Black, true);
				Serial.println(F("LED Module Configured."));

				configured = true;
			}
			else
			{
				Serial.println(F("\nERROR - failed to configure LED module"));
			}
		}
		else
		{
			Serial.println(F("\nERROR - failed to configure queue"));
		}
	}
	else
	{
		Serial.println(F("\nERROR - failed to configure WIFI"));
	}

	return configured;

} // end initialize

/**
 * calls others functions while we are not busy.
 * This is required since we disabled interrupts
 * to use the FastLED library.
 *
 * TODO: re-enable interrupts and see what happens
 *
 */
void worker()
{
	pubsubw.work(); // process queue
	yield(); // give time to ESP
	ESP.wdtFeed(); // pump watch dog
}


/**
 * Parses command buffer
 *
 */
void parseCommand()
{
	Command cmd;

	// Reset command available flag
	setCommandAvailable(false);
	gLedState = STATE_LED_COMMAND;
	gLedCounter = 0;

#ifdef __DEBUG
	Serial.print( millis() );
	Serial.print(F(" - parsing command..."));
#endif

	if( cmd.parse( (uint8_t *)pubsubw.getBuffer() ) )
	{
		if( cmd.getNodeId() == 0 )
		{
			cmd.setNodeId( config.getNodeId() );
		}
#ifdef __DEBUG
		cmd.dump();
#endif
		switch(cmd.getCommand())
		{
		case CMD_SHOW:
			Serial.println(F("SHOW"));
			controller.show();
			break;
		case CMD_SET_PIXEL:
			Serial.println(F("SET_PIXEL"));
			controller.setPixel(cmd.getIndex(), cmd.getOnColor(), cmd.getShow() );
			break;
		case CMD_FILL:
			Serial.println(F("FILL"));
			controller.fill(cmd.getOnColor(), true);
			break;
		case CMD_FILL_PATTERN:
			Serial.println(F("FILL_PATTERN"));
			controller.fillPattern(cmd.getPattern(), cmd.getOnColor(), cmd.getOffColor());
			break;
		case CMD_PATTERN:
			Serial.println(F("PATTERN"));
			controller.rotatePattern(cmd.getRepeat(),cmd.getDuration(), cmd.getPattern(), cmd.getDirection(), cmd.getOnColor(), cmd.getOffColor(), cmd.getOnTime(), cmd.getOffTime());
			break;
		case CMD_WIPE:
			Serial.println(F("WIPE"));
			controller.wipe(cmd.getRepeat(),cmd.getDuration(), cmd.getDirection(), cmd.getOnColor(), cmd.getOffColor(), cmd.getOnTime(), cmd.getOffTime(), cmd.getClearAfter(), cmd.getClearEnd());
			break;
		case CMD_SCROLL:
			Serial.println(F("SCROLL"));
			controller.scrollPattern(cmd.getRepeat(),cmd.getDuration(), cmd.getPattern(), cmd.getPatternLength(), cmd.getDirection(), cmd.getOnColor(), cmd.getOffColor(), cmd.getOnTime(), cmd.getOffTime(), cmd.getClearAfter(), cmd.getClearEnd());
			break;
		case CMD_BOUNCE:
			Serial.println(F("BOUNCE"));
			controller.bounce(cmd.getRepeat(), cmd.getDuration(), cmd.getPattern(), cmd.getPatternLength(), cmd.getDirection(), cmd.getOnColor(), cmd.getOffColor(), cmd.getOnTime(), cmd.getOffTime(), cmd.getBounceTime(), cmd.getClearAfter(), cmd.getClearEnd());
			break;
		case CMD_MIDDLE:
			controller.middle(cmd.getRepeat(),cmd.getDuration(), cmd.getDirection(), cmd.getOnColor(), cmd.getOffColor(), cmd.getOnTime(), cmd.getOffTime(), cmd.getClearAfter(), cmd.getClearEnd());
			Serial.println(F("MIDDLE"));
			break;
		case CMD_RANDOM_FLASH:
			Serial.println(F("RANDOM_FLASH"));
			controller.randomFlash(cmd.getRepeat(),cmd.getDuration(), cmd.getOnTime(), cmd.getOffTime(), cmd.getOnColor(), cmd.getOffColor());
			break;
		case CMD_FADE:
			Serial.println(F("FADE"));
			controller.fade(cmd.getDirection(), cmd.getFadeIncrement(), cmd.getFadeTime(), cmd.getOnColor());
			break;
		case CMD_STROBE:
			Serial.println(F("STROBE"));
			controller.strobe(cmd.getRepeat(),cmd.getDuration(), cmd.getOnColor(), cmd.getOffColor(), cmd.getOnTime(), cmd.getOffTime());
			break;
		case CMD_LIGHTNING:
			Serial.println(F("LIGHTNING"));
			controller.lightning(cmd.getRepeat(),cmd.getDuration(), cmd.getOnColor(), cmd.getOffColor());
			break;
		case CMD_STACK:
			Serial.println(F("STACK"));
			controller.stack(cmd.getRepeat(), cmd.getDuration(), cmd.getDirection(), cmd.getOnColor(), cmd.getOffColor(), cmd.getOnTime(), cmd.getClearEnd() );
			break;
		case CMD_FILL_RANDOM:
			Serial.println(F("RANDOM FILL"));
			controller.fillRandom(cmd.getRepeat(), cmd.getDuration(), cmd.getOnColor(), cmd.getOffColor(), cmd.getOnTime(), cmd.getOffTime(), cmd.getClearAfter(), cmd.getClearEnd());
			break;
		case CMD_RAINBOW:
			Serial.println(F("RAINBOW"));
			controller.rainbow(cmd.getDuration(), cmd.getProbability(), cmd.getOnColor(), cmd.getOnTime(), cmd.getHueUpdateTime());
			break;
		case CMD_RAINBOW_FADE:
			Serial.println(F("RAINBOW_FADE"));
			controller.rainbowFade(cmd.getDuration(), cmd.getOnTime());
			break;
		case CMD_CONFETTI:
			Serial.println(F("CONFETTI"));
			controller.confetti(cmd.getDuration(), cmd.getOnColor(), cmd.getFadeBy(), cmd.getOnTime(), cmd.getHueUpdateTime() );
			break;
		case CMD_CYLON:
			Serial.println(F("CYLON"));
			controller.cylon(cmd.getRepeat(), cmd.getDuration(), cmd.getOnColor(), cmd.getFadeTime(), cmd.getFramesPerSecond(), cmd.getHueUpdateTime());
			break;
		case CMD_BPM:
			Serial.println(F("BPM"));
			controller.bpm(cmd.getDuration(), cmd.getOnTime(), cmd.getHueUpdateTime() );
			break;
		case CMD_JUGGLE:
			Serial.println(F("JUGGLE"));
			controller.juggle(cmd.getDuration(), cmd.getOnTime());
			break;
		case CMD_SET_INTENSITY:
			Serial.println(F("SET_INTENSITY"));
			controller.setIntensity( cmd.getIntensity() );
			break;
		case CMD_COMPLETE:
			Serial.println(F("COMPLETE"));
			break;
		case CMD_ERROR:
		default:
			Serial.println(F("ERROR - UNKNOWN COMMAND"));
			break;
		} // end switch

		// Send response with the command is complete
		if( cmd.getNotifyOnComplete() )
		{
			if( cmd.buildResponse( pubsubw.getBuffer() ) )
			{
				Serial.print(F("Publishing Completion Response: "));
				Serial.println((char *)config.getMyResponseChannel() );

				// Publish message to response channel
				pubsubw.publish( (char *)config.getMyResponseChannel() );
			}

		} // end if notify on complete

		// NOTE: Don't be foolish and send a command to "all" with relay set; strange things will happen
		if( cmd.getRelay() == true )
		{
			Serial.print( millis() );
			Serial.println(F(" - Relay Command"));
			if(  cmd.getRelayNodeSize() > 0 )
			{
				uint8_t *relayNodes = cmd.getRelayNodes(); // get relay node array
				uint8_t destNode = relayNodes[0]; // save the node we are relaying to

				if( destNode > 0 )
				{
					cmd.setNodeId( destNode ); // change node ID for command

					Serial.print( millis() );
					Serial.print(F(" - Relaying to: "));
					Serial.println( destNode );

					// Shift relay nodes left 1, removing destination from list
					// (we don't want to relay back to ourselves)
					cmd.shiftRelayNodes();

					// Build "new" command to relay to next node
					if( cmd.buildCommand( pubsubw.getBuffer() ) )
					{
						// Build channel to send it to
						char channel[STRING_SIZE];
						memset(channel, 0, STRING_SIZE);
						sprintf((char *)channel, DEFAULT_CHANNEL_MY, destNode );

						// Send command
						Serial.print( millis() );
						Serial.print(F(" - Publishing Relay Command: "));
						Serial.println(channel );

						pubsubw.publish( channel );
					}
				}
				else
				{
					Serial.println(F("ERROR - Destination Node = 0"));
				}
			}
			else
			{
				Serial.println(F("** END OF RELAY CHAIN **"));
			}

		} // end if relay

	} // end if cmd parse = true

	gLedState = STATE_LED_WAITING;

	Serial.print( millis() );
	Serial.print(F(" - Command Complete"));

}

/**
 * Returns flag; true=new command is available
 */
boolean isCommandAvailable()
{
	return commandAvailable;
}

/**
 * Sets command available flag
 *
 */
void setCommandAvailable(boolean flag)
{
	commandAvailable = flag;
}


/**
 * Delay function with command check
 */
uint8_t commandDelay(uint32_t time)
{
	boolean cmd = isCommandAvailable();
	if( time == 0 ) return cmd;

	if( !cmd )
	{
		for (uint32_t i = 0; i < time; i++)
		{
			delay(1);
			worker();
			cmd = isCommandAvailable();
			if (cmd)
			{
				break;
			}
		}
	}
	return cmd;

}

/**
 * C function that calls class-level callback
 * Admittedly a hack, but it works
 *
 */
void pubsubCallback(char* topic, byte* payload, unsigned int length)
{
//	Serial.println("Calling pubsub callback...");
	pubsubw.callback(topic, payload, length);

}

/**
 * Software timer callback - toggles the LED
 *
 * Note: overly complicated but pleasing on the eyes ;)
 *
 */
void ledTimerCallback(void *pArg)
{
	gLedCounter += 1;

	switch( gLedState )
	{
	case STATE_LED_WAITING:
		switch( gLedCounter )
		{
		case 2:
			digitalWrite(BUILT_IN_LED, 0 );
			break;
		case 3:
			digitalWrite(BUILT_IN_LED, 1 );
			break;
		case 5:
			digitalWrite(BUILT_IN_LED, 0 );
			break;
		case 6:
			digitalWrite(BUILT_IN_LED, 1);
			break;
		case 12:
			gLedCounter = 0;
			break;
		default:
			break;
		}
		break;
	case STATE_LED_COMMAND:
		if( gLedCounter == 4 )
		{
			digitalWrite(BUILT_IN_LED, !digitalRead( BUILT_IN_LED ) );
			gLedCounter = 0;
		}
		break;
	case STATE_LED_ERROR:
		break;
	default:
		break;
	}

} // End of timerCallback
