/*
 *  Simple HTTP get webclient test
 */

//#define __DEBUG
#define __FASTLED
#define __LED_STRING
#define __ESP8266

#include "WifiNeoPixelClient.h"


// Wifi functions
uint8_t initializeWifi();

// mqtt functions
void pubsubCallback(char* topic, byte* payload, unsigned int length);
void connectQueue();

// internal functions
boolean isCommandAvailable();
void setCommandAvailable(boolean flag);
void parseCommand();
void commandMode();
void waitForConfig();
void readString(uint8_t *b);

// wifi variables
//const char* ssid = "*";
//const char* password = "*";
//const char* mqtt_server = "192.168.1.3";
WiFiClient wifi;

// mqtt variables
PubSubClient pubsub(wifi);

// led variables
NeopixelWrapper controller = NeopixelWrapper();

// Internal variables
static volatile uint8_t commandAvailable = false;
static volatile uint8_t commandBuffer[CMD_BUFFER_SIZE];
static volatile uint8_t nodeId = 0x01;
static uint32_t heartbeat = 0;
static Configuration config;

/**
 * Arduino setup function
 *
 */
void setup()
{
#ifdef __DEBUG
	Serial.begin(115200);
	Serial.println(F("\n\rESP8266 Powered Up"));
#endif

	// Basic HW setup
	pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
	digitalWrite(BUILTIN_LED, LOW);
	delay(2000);
	digitalWrite(BUILTIN_LED, HIGH);
	delay(2000);
	digitalWrite(BUILTIN_LED, LOW);
	delay(2000);

	Serial.println("Default Configuration:");
	config.dump();

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

	// Initialize wifi
	if( !initializeWifi() )
	{
		waitForConfig();
	}

	// Set up mqtt server
	pubsub.setServer( (const char *)config.getServerAddress(), 1883);
	pubsub.setCallback(pubsubCallback);

	if (controller.initialize(NUM_PIXELS, DEFAULT_INTENSITY) == false)
	{
		Serial.println(F("\nERROR - failed to configure LED module"));
		Helper::error( ERROR_DRIVER );
	}
	else
	{
		Serial.print(F("\nLED Controller initialized..."));
		controller.fill(CRGB::White, true);
		delay(500);
		controller.fill(CRGB::Black, true);
		delay(500);
		controller.fill(CRGB::Red, true);
		delay(500);
		controller.fill(CRGB::Black, true);
		delay(500);
		Serial.println(F("LED Module Configured."));
	}

    Serial.println("** Initialization Complete **");
	heartbeat = millis();

	digitalWrite(BUILTIN_LED, HIGH);

}

/**
 * Arduino loop function
 *
 */
void loop()
{
	// check if we are connected to mqtt server
	if (!pubsub.connected())
	{
		connectQueue();
	}

	// process mqtt info
	pubsub.loop();
	yield();

	if( isCommandAvailable() )
	{
		parseCommand();
	}

	if( Serial.available() )
	{
		commandMode();
	}

	if( heartbeat < millis() )
	{
		heartbeat = millis() + 500;
		digitalWrite(BUILTIN_LED, !digitalRead( BUILTIN_LED ) );
	}
}


/**
 * Handles serial commands and changing configuration
 */
void commandMode()
{
	uint8_t id = 0;
	uint8_t flag = 1;
	uint8_t b[STRING_SIZE+2];

	// Read first character - discard since it just gets us into command mode
	char c = toupper(Serial.read());

	while( flag )
	{
		Serial.println(F("\nCOMMAND MODE:"));
		Serial.println(F("C - Change Node ID"));
		Serial.println(F("I - Change Server IP"));
		Serial.println(F("S - Change SSID"));
		Serial.println(F("P - Change Password"));
		Serial.println(F("D - Dump Configuration"));
		Serial.println(F("D - Dump Configuration"));
		Serial.println(F("E - Save to Flash and Exit"));
		Serial.println(F("Q - Quit Without Saving to Flash"));
		while(!Serial.available() )
		{
			// process mqtt info
			pubsub.loop();
			yield();
		}

		c = toupper(Serial.read());
		switch( c )
		{
		case 'C':
			Serial.print(F("** Change Node Id **\nCurrent ID: "));
			Serial.println( config.getNodeId(), HEX );
			Serial.print(F("\nPlease enter the new node ID > "));
			while(!Serial.available() ){}
			id = Serial.read();
			Serial.println( (char)id ); // echo what the user typed
			Serial.println(F("\nProcessing...\n"));
			if( id >= '0' && id <= '9' )
			{
				config.setVersion( DEFAULT_VERSION );
				config.setNodeId( id-'0' );
				if( config.write() )
				{
					Serial.println(F("Node ID accepted."));
				}
				else
				{
					Serial.println(F("** ERROR - error saving configuration information."));
				}
			}
			else
			{
				Serial.print(F("Illegal node value entered:"));
				Serial.println(id);
			}
			break;
		case 'I':
			Serial.print(F("** Change Server Address**\nCurrent Address: "));
			Serial.println( (char *)config.getServerAddress() );
			Serial.print(F("\nPlease enter the new server address > "));
			readString( (uint8_t *)config.getServerAddress() );
			Serial.print("\nRead: ");
			Serial.println( (char *)config.getServerAddress() ) ;
			break;
		case 'S':
			Serial.print(F("** Change SSID **\nCurrent SSID: "));
			Serial.println( (char *)config.getSsid() );
			Serial.print(F("\nPlease enter the new SSID > "));
			readString( (uint8_t *)config.getSsid() );
			Serial.print("\nRead: ");
			Serial.println( (char *)config.getSsid() ) ;
			break;
		case 'P':
			Serial.print(F("** Change Password **\nCurrent Password: "));
			Serial.println( (char *)config.getPassword() );
			Serial.print(F("\nPlease enter the new password > "));
			readString( (uint8_t *)config.getPassword() );
			Serial.print("\nRead: ");
			Serial.println( (char *)config.getPassword() ) ;
			break;
		case 'D':
			Serial.println(F("\n** WIFI Configuration **"));
			WiFi.printDiag(Serial);
			Serial.println(F("\n** System Configuration **"));
			config.dump();
			break;
		case 'E':
			if( config.write() )
			{
				Serial.println("\nSaved configuration!\nExiting command mode...");
				flag = 0;
			}
			else
			{
				Serial.println("\nERROR - unable to write configuration.");
			}
			break;
		case 'Q':
			flag = 0;
			Serial.println("\nExiting command mode.");
			break;
		}
	}

}


void readString(uint8_t *b)
{
	uint8_t flag = 0;
	uint8_t c = 0;
	uint8_t i = 0;

	while (!flag)
	{
		while(!Serial.available() ){}
		c = Serial.read();
		if( c == '\r' || c == '\n')
		{
			b[i] = 0;
			flag = true;
		}
		else
		{
			b[i++] = c;
			Serial.print( (char)c ); // echo what the user typed
		}
	}

	while( Serial.available() )
	{
		Serial.read();
		Serial.println("Discarding character.");
	}

}

void waitForConfig()
{
	uint8_t count = 0;
	uint8_t flag = 1;

	// Read first character - discard since it just gets us into command mode
	char c = toupper(Serial.read());

	while( flag )
	{
		Helper::toggleLed(50);
		if( Serial.available() )
		{
			while(Serial.available() )
			{
			 Serial.read();
			}
			commandMode();
			if( initializeWifi() )
			{
				flag = true;
				break;
			}
			else
			{
				Helper::error(ERROR_WIRELESS);
			}
		}
	}
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

	if( cmd.parse( (uint8_t *)commandBuffer) )
	{
#ifdef __DEBUG
		Serial.println("command parsed:");
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
			break;
		case CMD_SET_FPS:
			Serial.println(F("SET_FPS"));
			break;
		case CMD_COMPLETE:
			Serial.println(F("COMPLETE"));
			break;
		case CMD_ERROR:
		default:
			Serial.println(F("ERROR"));
			break;
		} // end switch

		// Send response with the command is complete
		if( cmd.getNotifyOnComplete() )
		{
			char b[100];
			StaticJsonBuffer<100> jsonBuffer;
			JsonObject& root = jsonBuffer.createObject();
			// Our command is "complete"
			root[KEY_CMD] = CMD_COMPLETE;
			// Who is complete
			root[KEY_ID] = nodeId;
			// What command is complete
			root[KEY_CMD_RESPONSE] = cmd.getCommand();
			// Status of command (SUCCESS/FAIL)
			root[KEY_STATUS] = STATUS_SUCCESS;
			root.printTo(b, sizeof(b));
			pubsub.publish("crg/led/all", b);
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
			pubsub.loop(); // process mqtt loop
			yield(); // give esp8266 some time
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
 * Initializes WIFI on the ESP8266
 *
 */
uint8_t initializeWifi()
{

	uint8_t flag = false;
	uint8_t count = 0;

#ifdef __DEBUG
	Serial.println(F("Initializing WIFI:"));
	Serial.print(F("Connecting to "));
	Serial.print((char *)config.getSsid() );
#endif

	// We start by connecting to a WiFi network
	WiFi.begin( (char *)config.getSsid(), (char *)config.getPassword());

	while( count < config.getWifiTries() )
	{
		if( WiFi.status() == WL_CONNECTED )
		{
			flag = true;
			break;
		}
		else
		{
			count += 1;
			Serial.print(".");
			delay(500);
		}
	}

	if( flag )
	{
		Serial.print(F("- SUCCESS!\n\rConnected: "));
		Serial.println(WiFi.localIP());
	}
	else
	{
		Serial.print(F("- FAILED!"));
	}

	return flag;
}

/**
 * Handles all "receive" messages from MQTT
 *
 * Parses the messages and sends to correct handler
 *
 */
void pubsubCallback(char* topic, byte* payload, unsigned int length)
{
#ifdef __DEBUG
	Serial.print(F("Message arrived ["));
	Serial.print(topic);
	Serial.print(F("] "));
	for (uint8_t i = 0; i < length; i++)
	{
		Serial.print((char) payload[i]);
	}
	Serial.println();
#endif

	if( length <= CMD_BUFFER_SIZE )
	{
		memcpy( (void *)&commandBuffer, (void *)payload, length);
		setCommandAvailable(true);
	}
	else
	{
#ifdef __DEBUG
		Serial.println(F("ERROR - command buffer too small"));
#endif
	}

}

/**
 * Connects to MQTT to server
 *
 */
void connectQueue()
{
	// TODO - set max loop value before entering config

	// Loop until we're reconnected
	while (!pubsub.connected())
	{
#ifdef __DEBUG
		Serial.print(F("Attempting MQTT connection..."));
#endif
		// Attempt to connect
		if (pubsub.connect("ESP8266Client"))
		{
#ifdef __DEBUG
			Serial.println(F("connected. announcing presence.."));
#endif
			// Once connected, publish an announcement...
			pubsub.publish("crg/led/reg", "hello world");
			// subscribe to channels
			pubsub.subscribe("crg/led/all");
#ifdef __DEBUG
			Serial.println("Listening on 'all' channel.");
#endif
		}
		else
		{
#ifdef __DEBUG
			Serial.print(F("failed, rc="));
			Serial.print(pubsub.state());
			Serial.println(F(" try again in 5 seconds"));
#endif
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}


void testLeds()
{
//    // set the colors,
//    // if they don't match in order, you may need to use NEO_GRB flag
//    strip.SetPixelColor(0, red);
//    strip.SetPixelColor(1, green);
//    strip.SetPixelColor(2, blue);
//    strip.SetPixelColor(3, white);
//    strip.Show();
//
//
//    delay(3000);
//
//    // turn off the pixels
//    strip.SetPixelColor(0, black);
//    strip.SetPixelColor(1, black);
//    strip.SetPixelColor(2, black);
//    strip.SetPixelColor(3, black);
//    strip.Show();
//
//    delay(1000);
//
//    // set the colors,
//    // if they don't match in order, you may need to use NEO_GRB flag
//    strip.SetPixelColor(0, hslRed);
//    strip.SetPixelColor(1, hslGreen);
//    strip.SetPixelColor(2, hslBlue);
//    strip.SetPixelColor(3, hslWhite);
//    strip.Show();
//
//
//    delay(3000);
//
//    // turn off the pixels
//    strip.SetPixelColor(0, hslBlack);
//    strip.SetPixelColor(1, hslBlack);
//    strip.SetPixelColor(2, hslBlack);
//    strip.SetPixelColor(3, hslBlack);
//    strip.Show();

}
