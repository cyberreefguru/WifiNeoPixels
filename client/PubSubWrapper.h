/*
 * PubSubWrapper.h
 *
 *  Created on: Feb 21, 2016
 *      Author: tsasala
 */

#ifndef PUBSUBWRAPPER_H_
#define PUBSUBWRAPPER_H_
#include <Arduino.h>
#include <PubSubClient.h>

#include "ClientGlobal.h"
#include "Configuration.h"
#include "WifiWrapper.h"
#include "Command.h"
#include "Helper.h"

class PubSubWrapper
{
public:
	PubSubWrapper();
	uint8_t initialize(Configuration* config, WifiWrapper* wifi);
	void callback(char* topic, byte* payload, unsigned int length);

	uint8_t connect();
	uint8_t disconnect();
	uint8_t checkConnection();
	uint8_t connected();

	void work();
	void publish(char *channel);
	void publish( char *channel, char* buffer);
	void publish( char *channel, JsonObject& obj);
	uint8_t *getBuffer();


protected:
	PubSubClient pubsub;
	Configuration* config;
	uint8_t* cmdBuf;

};


//end of add your includes here
#ifdef __cplusplus
extern "C"
{
#endif

extern uint8_t isCommandAvailable();
extern void setCommandAvailable(uint8_t b);
extern uint8_t commandDelay(uint32_t time);

extern void pubsubCallback(char* topic, byte* payload, unsigned int length);

#ifdef __cplusplus
} // extern "C"
#endif


#endif /* PUBSUBWRAPPER_H_ */
