/*
 * Command.h
 *
 *  Created on: Jan 23, 2016
 *      Author: tsasala
 */

#ifndef COMMAND_H_
#define COMMAND_H_

#define __DEBUG

#include <Arduino.h>

#include <ArduinoJson.h>
#include "NeopixelWrapper.h"

#define CMD_BUFFER_SIZE		512
#define MAX_RELAY_NODES		16

// Defines JSON keys for command values
#define	KEY_CMD						"cmd"
#define KEY_NOTIFY_ON_COMPLETE		"noc"
#define KEY_RELAY					"rly"
#define KEY_RELAY_NODES				"rn"
#define KEY_FPS						"fps"
#define KEY_UPDATE_TIME				"udtime"
#define KEY_PATTERN					"p"
#define KEY_PATTERN_LENGTH			"pl"
#define KEY_DURATION				"d"
#define KEY_REPEAT					"r"
#define KEY_DIRECTION				"dir"
#define KEY_FADE_BY					"fb"
#define KEY_PROBABILITY				"pwin"
#define KEY_CLEAR_AFTER				"ca"
#define KEY_CLEAR_END				"ce"
#define KEY_ON_COLOR				"onc"
#define KEY_OFF_COLOR				"offc"
#define KEY_ON_TIME					"ont"
#define KEY_OFF_TIME				"offt"
#define KEY_BOUNCE_TIME				"bt"
#define KEY_FADE_TIME				"ft"
#define KEY_FADE_INCREMENT			"fi"
#define KEY_STATUS					"st"
#define KEY_NODE_ID					"nid"
#define KEY_INTENSITY				"i"
#define KEY_INDEX					"idx"
#define KEY_SHOW					"s"
#define KEY_UNIQUE_ID				"uid"

// Basic Functions
#define CMD_FILL                0x01	// Fills strip with specified color
#define CMD_FILL_PATTERN        0x02	// Fills strip with specified pattern
#define CMD_SET_PIXEL			0x03	// Sets specific pixel specified color
#define CMD_SHOW				0x04	// Shows pixels

// Basic Animations
#define CMD_PATTERN 			0x11	// Fills with pattern and rotates pattern
#define CMD_SCROLL			    0x12	// Scrolls pattern across strip
#define CMD_BOUNCE				0x13 	// Bounces pattern back and forth
#define CMD_MIDDLE				0x14	// Starts in middle and fills out
#define CMD_RANDOM_FLASH        0x15	// Randomly flashes pixels
#define	CMD_FADE				0x16	// Fades pixels
#define CMD_STROBE				0x17	// Rapid flash pixels
#define CMD_LIGHTNING			0x18	// Simulates lightning
#define CMD_STACK				0x19	// Progressively stacks pixels
#define CMD_FILL_RANDOM			0x1A	// Randomly fills pixels
#define CMD_WIPE				0x1B	// Wipes a single pixel


// Advanced Functions
#define CMD_RAINBOW             0x20
#define CMD_RAINBOW_FADE        0x21
#define CMD_CONFETTI            0x22
#define CMD_CYLON               0x23
#define CMD_BPM                 0x24
#define CMD_JUGGLE              0x25

// Administrative Functions
//#define CMD_SET_FPS             0x30
#define CMD_SET_HUE_UPDATE_TIME 0x31
#define CMD_SET_INTENSITY		0x32


// Other "commands"
#define CMD_COMPLETE			0x5E
#define CMD_ERROR               0x5F

// Error codes
#define STATUS_SUCCESS			0x01
#define STATUS_ERROR			0x10


class Command
{
public:
	Command();

	// Functions
	void initialize();
	uint8_t parse(uint8_t *b);
	uint8_t buildCommand(uint8_t *b);
	uint8_t buildResponse(uint8_t *b);
	void dump();

	// Getters and Setters
	uint8_t getCommand() const;
	void setCommand(uint8_t command);

	uint32_t getUniqueId() const;
	void setUniqueId(uint32_t uid);

	uint8_t getNodeId() const;
	void setNodeId(uint8_t nodeId);

	uint8_t getIndex() const;
	void setIndex(uint8_t index);

	uint8_t getShow() const;
	void setShow(uint8_t show);


	uint32_t getBounceTime() const;
	void setBounceTime(uint32_t bounceTime);
	uint8_t getClearAfter() const;
	void setClearAfter(uint8_t clearAfter);
	uint8_t getClearEnd() const;
	void setClearEnd(uint8_t clearEnd);
	uint8_t getDirection() const;
	void setDirection(uint8_t direction);
	uint32_t getDuration() const;
	void setDuration(uint32_t duration);
	uint8_t getFadeIncrement() const;
	void setFadeIncrement(uint8_t fadeIncrement);
	uint32_t getFadeTime() const;
	void setFadeTime(uint32_t fadeTime);
	uint8_t getFadeBy() const;
	void setFadeBy(uint8_t numOn);
	CRGB getOffColor() const;
	void setOffColor(uint32_t offColor);
	uint32_t getOffTime() const;
	void setOffTime(uint32_t offTime);
	CRGB getOnColor() const;
	void setOnColor(uint32_t onColor);
	uint32_t getOnTime() const;
	void setOnTime(uint32_t onTime);
	uint8_t getPattern() const;
	void setPattern(uint8_t pattern);

	uint8_t getPatternLength() const;
	void setPatternLength(uint8_t length);

	uint8_t getProbability() const;
	void setProbability(uint8_t probability);

	uint16_t getRepeat() const;
	void setRepeat(uint16_t repeat);

	uint8_t getNotifyOnComplete() const;
	void setNotifyOnComplete(uint8_t notifyOnComplete);

	uint8_t getRelay() const;
	void setRelay(uint8_t relay);

	uint8_t *getRelayNodes();
	void setRelayNode(uint8_t index, uint8_t value);
	uint8_t getRelayNodeSize();
	void shiftRelayNodes();

	uint8_t getIntensity() const;
	void setIntensity(uint8_t i);

	uint8_t getFramesPerSecond() const;
	void setFramesPerSecond(uint8_t framesPerSecond);

	uint8_t getUpdateTime() const;
	void setUpdateTime(uint8_t updateTime);

private:
	uint8_t command;
	uint32_t uniqueId;
	uint8_t nodeId;

	uint8_t notifyOnComplete;
	uint8_t relay;
	uint8_t relayNodes[MAX_RELAY_NODES];
	uint8_t relayNodeSize;

	uint8_t show;

	uint8_t framesPerSecond;
	uint8_t updateTime;
	uint8_t intensity;

	uint8_t index;
	uint8_t pattern;
	uint8_t patternLength;
	uint32_t duration;
	uint16_t repeat;
	uint8_t direction;
	uint8_t fadeBy;
	uint8_t probability;
	uint8_t clearAfter;
	uint8_t clearEnd;
	uint32_t onColor;
	uint32_t offColor;
	uint32_t onTime;
	uint32_t offTime;
	uint32_t bounceTime;
	uint32_t fadeTime;
	uint8_t fadeIncrement;

};

#endif /* COMMAND_H_ */
