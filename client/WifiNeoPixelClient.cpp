/*
 *  Simple HTTP get webclient test
 */

//#define __DEBUG

#include "WifiNeoPixelClient.h"

// Internal Variables
static Configuration config;
static NeopixelWrapper controller;
static PubSubWrapper pubsubw;
static WifiWrapper wifiw;
static Menu menu;

static volatile uint8_t commandAvailable = false;
static uint32_t heartbeat = 0;

void parseCommand();
boolean initialize();

//os_timer_t myTimer;

/**
 * Arduino setup function
 *
 */
void setup()
{
	// Configure serial port
	Serial.begin(115200);
	Serial.println(F("\n\rLED Controller Powered Up"));

	// Basic HW setup
	pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
	digitalWrite(BUILTIN_LED, LOW);
	delay(2000);
	yield();
	digitalWrite(BUILTIN_LED, HIGH);
	delay(2000);
	yield();
	digitalWrite(BUILTIN_LED, LOW);
	delay(2000);
	yield();

	Serial.println("\nInitializing configuration...");
    if( config.initialize() )
    {
        Serial.println("\nConfiguration initialized.");
    }
    else
    {
    	Serial.println("\n**ERROR - failed to initialize configuration");
    	Helper::error(ERROR_CONFIG);
    }

    // Initialize menu
	menu.initialize(&config);

	// Initialize wifi, pubsub, and LEDs
	if( !initialize() )
	{
		menu.waitForConfig();
		menu.configure();
		Serial.println(F("\nERROR - failed to initialize node."));
		Helper::error(ERROR_GENERAL);
	}

    Serial.println(F("** Initialization Complete **"));

	heartbeat = millis();

	digitalWrite(BUILTIN_LED, HIGH);

}

/**
 * Arduino loop function
 *
 */
void loop()
{
	uint8_t configFlag = 0;

	worker();

	// Check if command is available
	if( isCommandAvailable() )
	{
		parseCommand();
	}

	// check if we are connected to mqtt server
	// TODO: this pattern was from the sample code and I'm not sure it is necessary
	if( !pubsubw.checkConnection() )
	{
		menu.waitForConfig();
		configFlag = 1;
	}

	// Check if user wants to configure node
	if( Serial.available() )
	{
		configFlag = 1;
	}

	// flash LED
	if( heartbeat < millis() )
	{
		heartbeat = millis() + 500;
		digitalWrite(BUILTIN_LED, !digitalRead( BUILTIN_LED ) );
	}

	// Check if we need to enter command mode to reconfigure the node
	if( configFlag == 1)
	{
		configFlag = 0;
		menu.configure();
		Helper::error(ERROR_GENERAL);
	}

} // end loop

/**
 * Initializes the node.  Returns true if node configured properly, false otherwise.
 *
 */
boolean initialize()
{
	boolean configured = false;

	Serial.println(F("Configuring node..."));

	// Initialize WIFI
	if( wifiw.initialize(&config) )
	{
		// Initialize pubsub
		if( pubsubw.initialize(&config, &wifiw) )
		{
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

}

/**
 * calls others functions while we are not busy
 *
 */
void worker()
{
	pubsubw.work();
	yield();
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

#ifdef __DEBUG
		Serial.println("parsing command");
#endif

	if( cmd.parse( (uint8_t *)pubsubw.getBuffer() ) )
	{
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
			controller.rotatePattern(cmd.getRepeat(), cmd.getPattern(), cmd.getDirection(), cmd.getOnColor(), cmd.getOffColor(), cmd.getOnTime(), cmd.getOffTime());
			break;
		case CMD_WIPE:
			Serial.println(F("WIPE"));
			controller.scrollPattern(cmd.getPattern(), cmd.getDirection(), cmd.getOnColor(), cmd.getOffColor(), cmd.getOnTime(), cmd.getOffTime(), cmd.getClearAfter(), cmd.getClearEnd());
			break;
		case CMD_BOUNCE:
			Serial.println(F("BOUNCE"));
			controller.bounce(cmd.getRepeat(), cmd.getPattern(), cmd.getDirection(), cmd.getOnColor(), cmd.getOffColor(), cmd.getOnTime(), cmd.getOffTime(), cmd.getBounceTime(), cmd.getClearAfter(), cmd.getClearEnd());
			break;
		case CMD_MIDDLE:
			controller.middle(cmd.getRepeat(), cmd.getDirection(), cmd.getOnColor(), cmd.getOffColor(), cmd.getOnTime(), cmd.getOffTime(), cmd.getClearAfter(), cmd.getClearEnd());
			Serial.println(F("MIDDLE"));
			break;
		case CMD_RANDOM_FLASH:
			Serial.println(F("RANDOM_FLASH"));
			controller.randomFlash(0, cmd.getOnTime(), cmd.getOffTime(), cmd.getOnColor(), cmd.getOffColor());
			break;
		case CMD_FADE:
			Serial.println(F("FADE"));
			controller.fade(cmd.getDirection(), cmd.getFadeIncrement(), cmd.getFadeTime(), cmd.getOnColor());
			break;
		case CMD_STROBE:
			Serial.println(F("STROBE"));
			controller.strobe(cmd.getDuration(), cmd.getOnColor(), cmd.getOffColor(), cmd.getOnTime(), cmd.getOffTime());
			break;
		case CMD_LIGHTNING:
			Serial.println(F("LIGHTNING"));
			controller.lightning(cmd.getOnColor(), cmd.getOffColor());
			break;
		case CMD_RAINBOW:
			Serial.println(F("RAINBOW"));
			controller.rainbow(0, cmd.getProbability(), cmd.getOnColor(), cmd.getFramesPerSecond());
			break;
		case CMD_RAINBOW_FADE:
			Serial.println(F("RAINBOW_FADE"));
			controller.rainbowFade(0, cmd.getFramesPerSecond());
			break;
		case CMD_CONFETTI:
			Serial.println(F("CONFETTI"));
			controller.confetti(0, cmd.getOnColor(), cmd.getFadeBy(), cmd.getFramesPerSecond());
			break;
		case CMD_CYLON:
			Serial.println(F("CYLON"));
			controller.cylon(cmd.getRepeat(), cmd.getOnColor(), cmd.getFadeTime(), cmd.getFramesPerSecond());
			break;
		case CMD_BPM:
			Serial.println(F("BPM"));
			controller.bpm(0, cmd.getFramesPerSecond());
			break;
		case CMD_JUGGLE:
			Serial.println(F("JUGGLE"));
			controller.juggle(0, cmd.getFramesPerSecond());
			break;
		case CMD_SET_HUE_UPDATE_TIME:
			Serial.println(F("SET_HUE_UPDATE_TIME"));
			controller.setHueUpdateTime( cmd.getUpdateTime() );
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
			// TODO: figure out better way to handle this buffer size
			StaticJsonBuffer<100> jsonBuffer;
			JsonObject& root = jsonBuffer.createObject();

			// Our command is "complete"
			root[KEY_CMD] = CMD_COMPLETE;

			// Who is complete
			root[KEY_NODE_ID] = config.getNodeId();

			// What unique id of command that was complete
			root[KEY_UNIQUE_ID] = cmd.getUniqueId();

			// Status of command (SUCCESS/FAIL)
			root[KEY_STATUS] = STATUS_SUCCESS;
			pubsubw.publish( (char *)config.getMyResponseChannel(), root);
		}
	} // end if cmd parse = true

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

void pubsubCallback(char* topic, byte* payload, unsigned int length)
{
//	Serial.println("Calling pubsub callback...");
	pubsubw.callback(topic, payload, length);

}

