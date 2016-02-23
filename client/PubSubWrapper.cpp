/*
 * PubSubWrapper.cpp
 *
 *  Created on: Feb 21, 2016
 *      Author: tsasala
 */

#include "PubSubWrapper.h"

PubSubWrapper::PubSubWrapper()
{
	config = 0;
	commandBuffer = 0;
}

uint8_t PubSubWrapper::initialize(Configuration* config, WifiWrapper* wifi, uint8_t* commandBuffer)
{
	this->config = config;
	this->commandBuffer = commandBuffer;
	pubsub.setClient( wifi->getWifiClient() );

	// Set up mqtt server
	pubsub.setServer( (const char *)config->getServerAddress(), 1883);
	pubsub.setCallback(pubsubCallback);

	connect();

	return true;
}

/**
 * Handles all "receive" messages from MQTT
 *
 * Parses the messages and sends to correct handler
 *
 */
void PubSubWrapper::callback(char* topic, byte* payload, unsigned int length)
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
		memcpy( (void *)commandBuffer, (void *)payload, length);
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
uint8_t PubSubWrapper::connect()
{
	uint8_t count = 0;
	uint8_t flag = false;

	// Loop until we're reconnected or "timeout"
	while( count < config->getMqttTries() )
	{
		if( !pubsub.connected() )
		{
			Serial.print(F("Attempting queue connection..."));

			// Attempt to connect
			if (pubsub.connect("ESP8266Client"))
			{
				Serial.println(F("connected!"));

				// subscribe to channels
				pubsub.subscribe( (char *)config->getAllChannel() );
				pubsub.subscribe( (char *)config->getMyChannel() );

				// Once connected, publish an announcement...
				Serial.print(F("Announcing presence: "));
				Serial.println( (char *)config->getMyChannel() );

				// Tell controller we're listening
				pubsub.publish((char *)config->getRegistrationChannel(), (char *)config->getMyChannel() );

				flag = true;
				break;
			}
			else
			{
				Serial.print(F("failed! rc="));
				Serial.print(pubsub.state());
				Serial.println(F(" - try again in 1/2 second"));
				// Wait 500 ms before retrying
				delay(500);
			}

			count += 1;

		} // end if !connected
		else
		{
			flag = true;
		}

	} // end while

	if( !flag )
	{
		Serial.println("**ERROR - unable to bind to queue.");
	}

	return flag;
}

/**
 * Connects to MQTT to server
 *
 */
uint8_t PubSubWrapper::disconnect()
{
	uint8_t flag = false;

	if( pubsub.connected() )
	{
		Serial.print(F("Remove connections..."));
		pubsub.unsubscribe( (char *)config->getAllChannel() );
		pubsub.unsubscribe( (char *)config->getMyChannel() );
		pubsub.disconnect();
		flag = true;

	} // end if connected

	if( !flag )
	{
		Serial.println("**ERROR - unable to disconnect to queue.");
	}

	return flag;
}

uint8_t PubSubWrapper::checkConnection()
{
	uint8_t flag = false;
	if (!pubsub.connected())
	{
		if( connect() )
		{
			flag = true;
		}
	}
	else
	{
		flag = true;
	}

	return flag;
}

void PubSubWrapper::work()
{
	pubsub.loop();
}

void PubSubWrapper::publish(char *channel, char *buffer)
{
	pubsub.publish(channel, buffer);
}


/**
 * Waits in "error" mode until the user enters command mode and
 * configures the node.
 *
 */
void PubSubWrapper::waitForConfig()
{
	uint8_t flag = false;

	// Read characters - discard since it just gets us into command mode
	while(Serial.available() )
	{
	 Serial.read();
	}

	while( !flag )
	{
		Helper::toggleLed(50);
		if( Serial.available() )
		{
			while(Serial.available() )
			{
				Serial.read();
			}
//			commandMode();
			flag = true;
		}
	}
}

