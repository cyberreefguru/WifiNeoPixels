/*
 * Configuration.cpp
 *
 *  Created on: Oct 21, 2015
 *      Author: tsasala
 */

#include "Configuration.h"

const static char* defaultSsid = DEFAULT_SSID;
const static char* defaultPassword = DEFAULT_PASSWORD;
const static char* defaultServer = DEFAULT_SERVER;
const static char* defaultChannelAll = DEFAULT_CHANNEL_ALL;
const static char* defaultChannelMy = DEFAULT_CHANNEL_MY;
const static char* defaultChannelResp = DEFAULT_CHANNEL_RESP;

//const char* ssid = "*";
//const char* password = "*";
//const char* mqtt_server = "192.168.1.3";

Configuration::Configuration()
{
	initializeVariables();
}

uint8_t Configuration::initialize()
{
	uint8_t flag = false;

	EEPROM.begin(FLASH_SIZE);

	Serial.print(F("Reading client configuration...."));
	flag = read();
	if( flag)
	{
		Serial.print(F("checking version..."));
		if( getVersion() == DEFAULT_VERSION )
		{
			Serial.print(F("version matches: "));
			flag = true;
		}
		else
		{
			Serial.print(F("version mismatch: "));
		}
		Serial.println( getVersion(), HEX );
	}
	else
	{
		Serial.println(F("**ERROR - unable to read configuration."));
	}

	if( !flag )
	{
		Serial.print(F("Initializing configuration..."));
		initializeVariables();
		Serial.print(F("writing..."));
		flag = write();
		if( flag )
		{
			Serial.println(F("SUCCESS!"));
		}
		else
		{
			Serial.println(F("**FAILURE!"));
		}
	}

	dump();

	return flag;

}

uint8_t Configuration::getNodeId()
{
	return nodeId;
}

void Configuration::setNodeId(uint8_t nodeId)
{
	this->nodeId = nodeId;
}

uint8_t Configuration::getVersion()
{
	return version;
}

void Configuration::setVersion(uint8_t version)
{
	this->version = version;
}

uint8_t Configuration::getCrc()
{
	return crc;
}

void Configuration::setCrc(uint8_t crc)
{
	this->crc = crc;
}

const uint8_t* Configuration::getAllChannel() const
{
	return allChannel;
}

void Configuration::setAllChannel(uint8_t *b)
{
	strcpy( (char *)allChannel, (char *)b );
}

const uint8_t* Configuration::getMyChannel() const
{
	return myChannel;
}

void Configuration::setMyChannel(uint8_t *b)
{
	strcpy( (char *)myChannel, (char *)b );
}

const uint8_t* Configuration::getMyResponseChannel() const
{
	return myResponseChannel;
}

void Configuration::setMyResponseChannel(uint8_t *b)
{
	strcpy( (char *)myResponseChannel, (char *)b );
}

const uint8_t* Configuration::getServerAddress() const
{
	return serverAddress;
}

void Configuration::setServerAddress(uint8_t *b)
{
	strcpy( (char *)serverAddress, (char *)b );
}

const uint8_t* Configuration::getSsid() const
{
	return ssid;
}

void Configuration::setSsid(uint8_t *b)
{
	strcpy( (char *)ssid, (char *)b );
}


const uint8_t* Configuration::getPassword() const
{
	return password;
}

void Configuration::setPassword(uint8_t *b)
{
	strcpy( (char *)password, (char *)b );
}

void Configuration::dump()
{
	Serial.println(F("\nClient Configuration:"));
	Serial.print(F("Version         : "));
	Serial.println(version, HEX);
	Serial.print(F("Node ID         : "));
	Serial.println(nodeId, HEX);
	Serial.print(F("WIFI Tries      : "));
	Serial.println(wifiTries);

	Serial.print(F("SSID            : "));
	printBlock( ssid, STRING_SIZE );
	Serial.print(F("Password        : "));
	printBlock( password, STRING_SIZE );
	Serial.print(F("Server Address  : "));
	printBlock( serverAddress, STRING_SIZE );
	Serial.print(F("All Channel     : "));
	printBlock( allChannel, STRING_SIZE );
	Serial.print(F("My Channel      : "));
	printBlock( myChannel, STRING_SIZE );
	Serial.print(F("Response Channel: "));
	printBlock( myResponseChannel, STRING_SIZE );

	Serial.print(F("CRC             : "));
	Serial.println(crc, HEX);
}

/**
 * Reads the configuration
 *
 */
uint8_t Configuration::read()
{
	uint8_t flag = false;
	uint8_t checksum = 0;
	int address = CONFIG_START_ADDRESS;

	version = EEPROM.read(address++);
	nodeId = EEPROM.read(address++);;
	wifiTries = EEPROM.read(address++);;

	readBlock( ssid, (uint8_t *)address, STRING_SIZE );
	address += STRING_SIZE;
	readBlock( password, (uint8_t *)address, STRING_SIZE );
	address += STRING_SIZE;
	readBlock( serverAddress, (uint8_t *)address, STRING_SIZE );
	address += STRING_SIZE;
	readBlock( allChannel, (uint8_t *)address, STRING_SIZE );
	address += STRING_SIZE;
	readBlock( myChannel, (uint8_t *)address, STRING_SIZE );
	address += STRING_SIZE;
	readBlock(myResponseChannel, (uint8_t *)address, STRING_SIZE );
	address += STRING_SIZE;

	crc = EEPROM.read(address++);;

	checksum = computeChecksum((uint8_t *)&version, CRC_SIZE);
	if( checksum == crc )
	{
		flag = true;
	}
	else
	{
		flag = false;
		Serial.println("CRC FAILED");
	}

	return flag;

}

/**
 * Writes the configuration
 */
uint8_t Configuration::write()
{
	uint8_t flag = false;
	int address = CONFIG_START_ADDRESS;

	crc = computeChecksum((uint8_t *)&version, CRC_SIZE);

	EEPROM.write(address++, version);
	EEPROM.write(address++, nodeId);
	EEPROM.write(address++, wifiTries);
	writeBlock( (uint8_t *)address, ssid, STRING_SIZE );
	address += STRING_SIZE;
	writeBlock( (uint8_t *)address, password, STRING_SIZE );
	address += STRING_SIZE;
	writeBlock( (uint8_t *)address, serverAddress, STRING_SIZE );
	address += STRING_SIZE;
	writeBlock( (uint8_t *)address, allChannel, STRING_SIZE );
	address += STRING_SIZE;
	writeBlock( (uint8_t *)address, myChannel, STRING_SIZE );
	address += STRING_SIZE;
	writeBlock( (uint8_t *)address, myResponseChannel, STRING_SIZE );
	address += STRING_SIZE;
	EEPROM.write(address, crc);

	if( !EEPROM.commit() )
	{
		Serial.println("**ERROR - failed to commit to flash");
	}
	else
	{
		flag = ( EEPROM.read(address) == crc );
		if( flag == false )
		{
			Serial.println("**ERROR - flash CRC does not match in-memory CRC.");
		}
	}

	return flag;

}

void Configuration::readBlock(uint8_t *d, uint8_t *s, uint8_t len)
{
	for(uint8_t i=0; i<len; i++)
	{
		d[i] = EEPROM.read((int)&s[i]);;
	}
}


void Configuration::writeBlock(uint8_t *d, uint8_t *s, uint8_t len )
{
	for(uint8_t i=0; i<len; i++)
	{
		EEPROM.write( (int)&d[i], (uint8_t)s[i]);
	}

}

void Configuration::printBlock(uint8_t *s, uint8_t len )
{
	for(uint8_t i=0; i<len; i++)
	{
		Serial.print( (char)s[i] );
	}
	Serial.println();
}

uint8_t Configuration::getWifiTries() const
{
	return wifiTries;
}

void Configuration::setWifiTries(uint8_t wifiTries)
{
	this->wifiTries = wifiTries;
}

void Configuration::initializeVariables()
{
	version = DEFAULT_VERSION;
	nodeId = DEFAULT_NODE_ID;
	wifiTries = DEFAULT_WIFI_TRIES;

	memset(ssid, 0, STRING_SIZE);
	strcpy( (char *)ssid, defaultSsid);
	memset(password, 0, STRING_SIZE);
	strcpy( (char *)password, defaultPassword);
	memset(serverAddress, 0, STRING_SIZE);
	strcpy( (char *)serverAddress, defaultServer);
	memset(allChannel, 0, STRING_SIZE);
	strcpy( (char *)allChannel, defaultChannelAll);
	memset(myChannel, 0, STRING_SIZE);
	strcpy( (char *)myChannel, defaultChannelMy);
	memset(myResponseChannel, 0, STRING_SIZE);
	strcpy( (char *)myResponseChannel, defaultChannelResp);

	crc = 0;
}

/**
 * Computes CRC8 using data provided.
 *
 * CRC-8 - based on the CRC8 formulas by Dallas/Maxim
 * code released under the terms of the GNU GPL 3.0 license
 */
uint8_t Configuration::computeChecksum(uint8_t *data, uint8_t len)
{
	uint8_t crc = 0x00;
	while (len--)
	{
		uint8_t extract = *data++;
		for (uint8_t tempI = 8; tempI; tempI--)
		{
			uint8_t sum = (crc ^ extract) & 0x01;
			crc >>= 1;
			if (sum)
			{
				crc ^= 0x8C;
			}
			extract >>= 1;
		}
	}
	return crc;
}

