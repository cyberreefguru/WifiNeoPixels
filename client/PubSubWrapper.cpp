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
	jsonBuffer = 0;
}

uint8_t PubSubWrapper::initialize(Configuration* config, WifiWrapper* wifi)
{
	this->config = config;
	pubsub.setClient( wifi->getWifiClient() );

	// Check if we are connected - if so, disconnect
	if( pubsub.connected() )
	{
		disconnect();
	}

	// Set up mqtt server
	pubsub.setServer( (const char *)config->getServerAddress(), 1883);
	pubsub.setCallback(pubsubCallback);

	// Check if we already allocated memory; if so, free it
	if( jsonBuffer != 0 )
	{
		free(jsonBuffer);
	}

	// Allocate memory
	jsonBuffer = (uint8_t *)malloc(JSON_BUFFER_SIZE);
	if( jsonBuffer == 0 )
	{
		Serial.println(F("ERROR - unable to allocate json buffer memory!"));
		return false;
	}

	return connect();
}

/**
 * Connects to MQTT to server
 *
 */
uint8_t PubSubWrapper::connect()
{
	uint8_t count = 0;
	uint8_t flag = false;

	if( pubsub.connected() )
	{
		Serial.println(F("Queue connected."));
	}
	else
	{
		Serial.print(F("Attempting queue connection..."));

		// Loop until we're reconnected or "timeout"
		while( count < config->getMqttTries() )
		{
			// Attempt to connect
			if (pubsub.connect((char *)config->getMyChannel()))
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
				Serial.print(F("."));
				// Wait 500 ms before retrying
				delay(500);
			}
			count += 1;

		} // end while
	}

	if( !flag )
	{
		Serial.println(F("\n**ERROR - unable to bind to queue."));
	}

	return flag;
}

/**
 * Disconnects from MQTT server
 *
 */
uint8_t PubSubWrapper::disconnect()
{
	uint8_t flag = false;

	if( pubsub.connected() )
	{
		Serial.print(F("Disconnecting..."));
		pubsub.unsubscribe( (char *)config->getAllChannel() );
		pubsub.unsubscribe( (char *)config->getMyChannel() );
		pubsub.disconnect();
		Serial.println(F("success!"));
		flag = true;

	}
	else
	{
		Serial.println(F("ERROR - not connected"));
	}

	return flag;
}

/**
 * Checks if we still have a valid connection to the server.
 * If there is no connection, an attempt will be made.
 *
 */
uint8_t PubSubWrapper::checkConnection()
{
	uint8_t flag = pubsub.connected();

	if (!flag)
	{
		flag = connect();
	}

	return flag;
}

/**
 * Calls the loop function to process queue messages
 *
 */
void PubSubWrapper::work()
{
	pubsub.loop();
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
		// Copy payload to command buffer
		memcpy( (void *)jsonBuffer, (void *)payload, length);
		setCommandAvailable(true);
	}
	else
	{
		Serial.println(F("ERROR - command buffer too small"));
	}

}


/**
 * Publishes a message to the specified channel
 *
 */
void PubSubWrapper::publish(char *channel, char *buffer)
{
	pubsub.publish(channel, buffer);
}

/**
 * Publishes a message to the specified channel
 *
 */
void PubSubWrapper::publish(char *channel, JsonObject& obj)
{
	obj.printTo((char *)jsonBuffer, JSON_BUFFER_SIZE);
	pubsub.publish( channel, (char *)jsonBuffer );
}


/**
 * Returns a pointer to the internal buffer
 *
 */
uint8_t* PubSubWrapper::getBuffer()
{
	return jsonBuffer;
}

