/*
 * Menu.cpp
 *
 *  Created on: Feb 21, 2016
 *      Author: tsasala
 */

#include "Menu.h"

Menu::Menu()
{
	this->config = 0;
//	this->wifi = 0;
//	this->pubsub = 0;
//	this->controller = 0;
}

//uint8_t Menu::initialize(Configuration* config, WifiWrapper* wifi, PubSubWrapper* pubsub, NeopixelWrapper* controller)
uint8_t Menu::initialize(Configuration* config)
{
	uint8_t flag = false;

	this->config = config;
//	this->wifi = wifi;
//	this->pubsub = pubsub;
//	this->controller = controller;
	flag = true;

	Serial.println( F("Menu configured.") );
	return flag;
}

/**
 * Uses stdout/stdin to configure the node
 *
 */
uint8_t Menu::configure()
{
	uint8_t b[INPUT_BUFFER_SIZE];
	int16_t id = 0;
	uint8_t flag = 1;
	uint8_t changed = 0;
	uint8_t saved = 0;
	int8_t c = 0;

	// Read and discard input data since it just gets us into command mode
	Helper::readAll();

	while (flag)
	{
		// Print menu
		Serial.println(F("\nCOMMAND MODE:"));
		Serial.println(F("1 - Change Node ID"));
		Serial.println(F("2 - Number of LEDs"));
		Serial.println(F("3 - Change Server IP"));
		Serial.println(F("4 - Change SSID"));
		Serial.println(F("5 - Change Password"));
		Serial.println(F("6 - All Channel"));
		Serial.println(F("7 - Registration Channel"));
		Serial.println(F("8 - Node Channel"));
		Serial.println(F("9 - Response Channel"));

		Serial.println(F("\nD - Dump Configuration"));
		Serial.println(F("E - Save to Flash"));
		Serial.println(F("Q - Quit"));

		c = toupper( Helper::readChar(false) );
		switch ( c )
		{
		case '1':
			Serial.print(F("** Change Node Id **\nCurrent ID: "));
			Serial.println(config->getNodeId(), HEX);
			Serial.print(F("\nPlease enter the new node ID (1-255) > "));
			id = Helper::readInt(b, INPUT_BUFFER_SIZE);
			if (id > 0 && id <= 255)
			{
				config->setNodeId( id );
				Serial.print(F("\nNodeID: "));
				Serial.println(config->getNodeId(), HEX);
				changed = 1;
			}
			else
			{
				Serial.print(F("\nERROR - illegal node value entered: "));
				Serial.println(id);
			}
			break;
		case '2':
			Serial.print(F("** Change Number of LEDs**\nCurrent Number: "));
			Serial.println(config->getNumberLeds());
			Serial.print(F("\nPlease enter the new number (1-255) > "));
			id = Helper::readInt(b, INPUT_BUFFER_SIZE);
			if (id > 0 && id <= 255)
			{
				config->setNumberLeds(id);
				Serial.print(F("\nNumber of LEDs: "));
				Serial.println(config->getNumberLeds());
				changed = 1;
				//controller->initialize(config->getNumberLeds(), DEFAULT_INTENSITY);
			}
			else
			{
				Serial.print(F("\nERROR - illegal value entered: "));
				Serial.println(id);
			}
			break;
		case '3':
			Serial.print(F("** Change Server Address**\nCurrent Address: "));
			Serial.println((char *) config->getServerAddress());
			Serial.print(F("\nPlease enter the new server address > "));
			if( Helper::readString(b, INPUT_BUFFER_SIZE) > 0 )
			{
				memcpy( (char *)config->getServerAddress(), (char *)b, INPUT_BUFFER_SIZE );
				Serial.print(F("\nServer Address: "));
				Serial.println((char *) config->getServerAddress());
				changed = 1;
			}
			else
			{
				Serial.print(F("\nNo change!"));
			}
			break;
		case '4':
			Serial.print(F("** Change SSID **\nCurrent SSID: "));
			Serial.println((char *) config->getSsid());
			Serial.print(F("\nPlease enter the new SSID > "));
			if( Helper::readString(b, INPUT_BUFFER_SIZE) > 0 )
			{
				memcpy( (char *)config->getSsid(), (char *)b, INPUT_BUFFER_SIZE );
				Serial.print(F("\nSSID: "));
				Serial.println((char *) config->getSsid());
				changed = 1;
			}
			else
			{
				Serial.print(F("\nNo change!"));
			}
			break;
		case '5':
			Serial.print(F("** Change Password **\nCurrent Password: "));
			Serial.println((char *) config->getPassword());
			Serial.print(F("\nPlease enter the new password > "));
			if( Helper::readString(b, INPUT_BUFFER_SIZE) > 0 )
			{
				memcpy( (char *)config->getPassword(), (char *)b, INPUT_BUFFER_SIZE );
				Serial.print(F("\nPassword: "));
				Serial.println((char *) config->getPassword());
				changed = 1;
			}
			else
			{
				Serial.print(F("\nNo change!"));
			}
			break;

		case '6':
			Serial.print(F("** Change All Channel **\nCurrent All Channel: "));
			Serial.println((char *) config->getAllChannel());
			Serial.print(F("\nPlease enter the channel > "));
			if( Helper::readString(b, INPUT_BUFFER_SIZE) > 0 )
			{
				memcpy( (char *)config->getAllChannel(), (char *)b, INPUT_BUFFER_SIZE );
				Serial.print(F("\nAll Channel: "));
				Serial.println((char *) config->getAllChannel());
				changed = 1;
			}
			else
			{
				Serial.print(F("\nNo change!"));
			}
			break;

		case '7':
			Serial.print(F("** Change Registration Channel **\nCurrent Registration Channel: "));
			Serial.println((char *) config->getRegistrationChannel());
			Serial.print(F("\nPlease enter the channel > "));
			if( Helper::readString(b, INPUT_BUFFER_SIZE) > 0 )
			{
				memcpy( (char *)config->getRegistrationChannel(), (char *)b, INPUT_BUFFER_SIZE );
				Serial.print(F("\nRegistration Channel: "));
				Serial.println((char *) config->getRegistrationChannel());
				changed = 1;
			}
			else
			{
				Serial.print(F("\nNo change!"));
			}
			break;
		case '8':
			Serial.print(F("** Change Node Channel **\nCurrent Node Channel: "));
			Serial.println((char *) config->getMyChannel());
			Serial.print(F("\nPlease enter the channel > "));
			if( Helper::readString(b, INPUT_BUFFER_SIZE) > 0 )
			{
				memcpy( (char *)config->getMyChannel(), (char *)b, INPUT_BUFFER_SIZE );
				Serial.print(F("\nNode Channel: "));
				Serial.println((char *) config->getMyChannel());
				changed = 1;
			}
			else
			{
				Serial.print(F("\nNo change!"));
			}
			break;

		case '9':
			Serial.print(F("** Change Response Channel **\nCurrent Response Channel: "));
			Serial.println((char *) config->getMyResponseChannel());
			Serial.print(F("\nPlease enter the channel > "));
			if( Helper::readString(b, INPUT_BUFFER_SIZE) > 0 )
			{
				memcpy( (char *)config->getMyResponseChannel(), (char *)b, INPUT_BUFFER_SIZE );
				Serial.print(F("\nResponse Channel: "));
				Serial.println((char *) config->getMyResponseChannel());
				changed = 1;
			}
			else
			{
				Serial.print(F("\nNo change!"));
			}
			break;

		case 'D':
			Serial.println(F("\n** WIFI Configuration **"));
			WiFi.printDiag(Serial);
			Serial.println(F("\n** System Configuration **"));
			config->dump();
			break;
		case 'E':
			if (config->write())
			{
				Serial.println(F("\nSaved configuration!"));
				saved = 1;
			}
			else
			{
				Serial.println(F("\nERROR - unable to write configuration."));
			}
			break;
		case 'Q':
			// Check if we have unsaved changes
			if( changed == 1 && saved == 0 )
			{
				Serial.println("Unsaved changes! Save now (Y/N)? ");

				c = toupper( Helper::readChar(false) );
				if( c == 'Y' )
				{
					if (config->write())
					{
						Serial.println(F("\nSaved configuration!"));
						saved = 1;
					}
					else
					{
						Serial.println(F("\nERROR - unable to write configuration."));
					}
				}
				else
				{
					Serial.println(F("\nExiting without saving."));
					changed = 0;
				}
			}
			Serial.println(F("\nQuitting command mode."));
			flag = 0;
			break;
		default:
			break;
		} // end case

	} // end while

	return changed;

} // end configure

/**
 * Waits in "error" mode until the user enters command mode and
 * configures the node.
 *
 */
void Menu::waitForConfig()
{
	// Read characters and discard
	Helper::readAll();

	while (1)
	{
		Helper::toggleLed(50);

		// Wait until user initiates command mode by pressing keys
		if (Serial.available())
		{
			break;
		}
	}

	// Read characters - discard since it just gets us into command mode
	Helper::readAll();

}
