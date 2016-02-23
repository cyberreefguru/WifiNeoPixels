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
	this->wifi = 0;
	this->pubsub = 0;
	this->controller = 0;
}

uint8_t Menu::initialize(Configuration* config, WifiWrapper* wifi, PubSubWrapper* pubsub, NeopixelWrapper* controller)
{
	uint8_t flag = false;

	this->config = config;
	this->wifi = wifi;
	this->pubsub = pubsub;
	this->controller = controller;
	flag = true;

	return flag;
}

/**
 * Uses stdout/stdin to configure the node
 *
 */
void Menu::configure()
{
	uint8_t id = 0;
	uint8_t flag = 1;
	char c;
	uint8_t buf[4];

	// Read first character - discard since it just gets us into command mode
	while (Serial.available())
	{
		c = Serial.read();
	}

	while (flag)
	{
		Serial.println(F("\nCOMMAND MODE:"));
		Serial.println(F("C - Change Node ID"));
		Serial.println(F("L - Number of LEDs"));
		Serial.println(F("I - Change Server IP"));
		Serial.println(F("S - Change SSID"));
		Serial.println(F("P - Change Password"));
		Serial.println(F("D - Dump Configuration"));
		Serial.println(F("E - Save to Flash and Exit"));
		Serial.println(F("Q - Quit Without Saving to Flash"));
		while (!Serial.available())
		{
			worker();
		}

		c = toupper(Serial.read());
		switch (c)
		{
		case 'C':
			Serial.print(F("** Change Node Id **\nCurrent ID: "));
			Serial.println(config->getNodeId(), HEX);
			Serial.print(F("\nPlease enter the new node ID > "));
			while (!Serial.available())
			{
			}
			id = Serial.read();
			Serial.println((char) id); // echo what the user typed
			Serial.println(F("\nProcessing...\n"));
			if (id >= '0' && id <= '9')
			{
				config->setVersion( DEFAULT_VERSION);
				config->setNodeId(id - '0');
				pubsub->disconnect();
				//disconnectPubSub();
				if (config->write())
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
		case 'L':
			Serial.print(F("** Change Number of LEDs**\nCurrent Number: "));
			Serial.println(config->getNumberLeds());
			Serial.print(F("\nPlease enter the new number (1-256) > "));
			Helper::readString(buf, 6);
			id = (uint8_t) atoi((char *) buf);
			Serial.print("Read: ");
			Serial.println(id);

			if (id > 0 && id <= 256)
			{
				config->setNumberLeds(id);
				controller->reinitialize(config->getNumberLeds(), DEFAULT_INTENSITY);
			}
			else
			{
				Serial.println("**Error converting input to a valid number");
			}

			break;
		case 'I':
			Serial.print(F("** Change Server Address**\nCurrent Address: "));
			Serial.println((char *) config->getServerAddress());
			Serial.print(F("\nPlease enter the new server address > "));
			Helper::readString((uint8_t *) config->getServerAddress(), STRING_SIZE);
			Serial.print("Read: ");
			Serial.println((char *) config->getServerAddress());
			break;
		case 'S':
			Serial.print(F("** Change SSID **\nCurrent SSID: "));
			Serial.println((char *) config->getSsid());
			Serial.print(F("\nPlease enter the new SSID > "));
			Helper::readString((uint8_t *) config->getSsid(), STRING_SIZE);
			Serial.print("Read: ");
			Serial.println((char *) config->getSsid());
			break;
		case 'P':
			Serial.print(F("** Change Password **\nCurrent Password: "));
			Serial.println((char *) config->getPassword());
			Serial.print(F("\nPlease enter the new password > "));
			Helper::readString((uint8_t *) config->getPassword(), STRING_SIZE);
			Serial.print("Read: ");
			Serial.println((char *) config->getPassword());
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
		default:
			break;
		}
	}
}

/**
 * Waits in "error" mode until the user enters command mode and
 * configures the node.
 *
 */
void Menu::waitForWifiConfig()
{
	uint8_t flag = 1;

	// Read characters - discard since it just gets us into command mode
	while (Serial.available())
	{
		Serial.read();
		yield();
	}

	while (flag)
	{
		Helper::toggleLed(50);
		yield();

		if (Serial.available())
		{
			while (Serial.available())
			{
				Serial.read();
			}
			configure();
			if (wifi->initialize())
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
 * Waits in "error" mode until the user enters command mode and
 * configures the node.
 *
 */
void Menu::waitForPubSubConfig()
{
	uint8_t flag = false;

	// Read characters - discard since it just gets us into command mode
	while (Serial.available())
	{
		Serial.read();
	}

	while (!flag)
	{
		Helper::toggleLed(50);
		if (Serial.available())
		{
			while (Serial.available())
			{
				Serial.read();
			}
			configure();
			flag = true;
		}
	}
}

