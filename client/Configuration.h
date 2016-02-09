/*
 * Configuration.h
 *
 *  Created on: Oct 21, 2015
 *      Author: tsasala
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <Arduino.h>
#include <EEPROM.h>

#define CONFIG_START_ADDRESS	0x00
#define CONFIG_V1				0x01
#define CONFIG_V2				0x02
#define CONFIG_V3				0x03

#define DEFAULT_VERSION			CONFIG_V3
#define DEFAULT_NODE_ID			'1'
#define DEFAULT_NUMBER_NODES	0x06

#define DEFAULT_WIFI_TRIES		20
#define DEFAULT_MQTT_TRIES		20

#define DEFAULT_SSID			"ssid"
#define DEFAULT_PASSWORD		"password"
#define DEFAULT_SERVER			"192.168.1.3"
#define DEFAULT_CHANNEL_ALL		"crg/led/all"
#define DEFAULT_CHANNEL_MY		"crg/led/node/"
#define DEFAULT_CHANNEL_RESP	"crg/led/resp/"


#define STRING_SIZE				20
#define FLASH_SIZE				150
#define CRC_SIZE				124

class Configuration
{
public:
	Configuration();
	uint8_t initialize();

	uint8_t getVersion();
	void setVersion(uint8_t v);
	uint8_t getNodeId();
	void setNodeId(uint8_t v);

	uint8_t getWifiTries() const;
	void setWifiTries(uint8_t wifiTries);

	uint8_t getMqttTries() const;
	void setMqttTries(uint8_t tries);

	const uint8_t* getAllChannel() const;
	void setAllChannel(uint8_t *b);

	const uint8_t* getMyChannel() const;
	void setMyChannel(uint8_t *b);

	const uint8_t* getMyResponseChannel() const;
	void setMyResponseChannel(uint8_t *b);

	const uint8_t* getServerAddress() const;
	void setServerAddress(uint8_t *b);

	const uint8_t* getSsid() const;
	void setSsid(uint8_t *b);

	const uint8_t* getPassword() const;
	void setPassword(uint8_t *b);

	uint8_t getCrc();
	void setCrc(uint8_t v);

	uint8_t write();
	uint8_t read();
	void dump();

protected:
	uint8_t computeChecksum(uint8_t *data, uint8_t len);

	uint8_t version;
	uint8_t nodeId;
	uint8_t wifiTries;
	uint8_t mqttTries;
	uint8_t ssid[STRING_SIZE];
	uint8_t password[STRING_SIZE];
	uint8_t serverAddress[STRING_SIZE];
	uint8_t allChannel[STRING_SIZE];
	uint8_t myChannel[STRING_SIZE];
	uint8_t myResponseChannel[STRING_SIZE];
	uint8_t crc;

	void readBlock(uint8_t *d, uint8_t *s, uint8_t len);
	void writeBlock(uint8_t *d, uint8_t *s, uint8_t len);
	void printBlock(uint8_t *s, uint8_t len );
	void initializeVariables();

};

#endif /* CONFIGURATION_H_ */
