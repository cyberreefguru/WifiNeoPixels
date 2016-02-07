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

#define DEFAULT_VERSION			CONFIG_V1
#define DEFAULT_NODE_ID			'1'
#define DEFAULT_NUMBER_NODES	0x06

#define CRC_SIZE				0x03


class Configuration
{
public:
	Configuration();
	uint8_t initialize();
	uint8_t getVersion();
	void setVersion(uint8_t v);
	uint8_t getNodeId();
	void setNodeId(uint8_t v);
	uint8_t getNumberNodes();
	void setNumberNodes(uint8_t numberNodes);
	uint8_t getCrc();
	void setCrc(uint8_t v);

	uint8_t write();
	uint8_t read();
	void dump();


protected:
	uint8_t computeChecksum(uint8_t *data, uint8_t len);

	uint8_t version;
	uint8_t nodeId;
	uint8_t numberNodes;
	uint8_t queueIpAddress[16];
	uint8_t allChannel[16];
	uint8_t regChannel[16];
	uint8_t myChannel[16];
	uint8_t myResponseChannel[16];
	uint8_t crc;

};

#endif /* CONFIGURATION_H_ */
