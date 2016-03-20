/*
 * Command.cpp
 *
 *  Created on: Jan 23, 2016
 *      Author: tsasala
 */

#include "Command.h"



/**
 * Constructor - initializes object to defaults
 *
 */
Command::Command()
{
	initialize();
}

/**
 * Initializes the object to defaults
 *
 */
void Command::initialize()
{
	command = CMD_ERROR;
	uniqueId = 0;
	notifyOnComplete = false;
	relay = false;
	framesPerSecond = DEFAULT_FPS;
	updateTime = 0;
	intensity = DEFAULT_INTENSITY;
	pattern = 0;
	repeat = 0;
	direction = RIGHT;
	duration = 0;
	fadeBy = 1;
	probability = 0;
	clearAfter = false;
	clearEnd = false;
	onColor = WHITE;
	offColor = BLACK;
	onTime = 0;
	offTime = 0;
	bounceTime = 0;
	fadeTime = 0;
	fadeIncrement = 0;
	index = 0;
	show = false;

	for(uint8_t i=0; i<MAX_RELAY_NODES; i++)
	{
		relayNodes[i] = 0;
	}

} // end initialize

/**
 * Parses buffer into object
 *
 */
uint8_t Command::parse(uint8_t* b)
{
	uint8_t status = 0;

#ifdef __DEBUG
	Serial.println("Parsing buffer...");
#endif

	StaticJsonBuffer<CMD_BUFFER_SIZE> jsonBuffer;
	JsonObject& obj = jsonBuffer.parseObject((char *)b);
	if (!obj.success())
	{
#ifdef __DEBUG
		Serial.print(F("parseObject() failed: "));
		Serial.println(obj.success());
#endif
		return status;
	}
	else
	{
#ifdef __DEBUG
		Serial.println("Successful parse; pulling values...");
#endif

		command = obj[KEY_CMD].as<uint8_t>();
		uniqueId = obj[KEY_UNIQUE_ID].as<long>();
		nodeId = obj[KEY_NODE_ID].as<uint8_t>();
		notifyOnComplete = obj[KEY_NOTIFY_ON_COMPLETE].as<uint8_t>();
		relay = obj[KEY_RELAY].as<uint8_t>();
		show = obj[KEY_SHOW].as<uint8_t>();

		framesPerSecond = obj[KEY_FPS].as<uint8_t>();
		updateTime = obj[KEY_UPDATE_TIME].as<uint8_t>();
		intensity = obj[KEY_INTENSITY].as<uint8_t>();

		index = obj[KEY_INDEX].as<uint8_t>();
		pattern = obj[KEY_PATTERN].as<uint8_t>();
		patternLength = obj[KEY_PATTERN_LENGTH].as<uint8_t>();
		duration = obj[KEY_DURATION].as<uint32_t>();
		repeat = obj[KEY_REPEAT].as<uint16_t>();
		direction = obj[KEY_DIRECTION].as<uint8_t>();
		fadeBy = obj[KEY_FADE_BY].as<uint8_t>();
		probability = obj[KEY_PROBABILITY].as<uint8_t>();
		clearAfter = obj[KEY_CLEAR_AFTER].as<uint8_t>();
		clearEnd = obj[KEY_CLEAR_END].as<uint8_t>();

		onColor = obj[KEY_ON_COLOR].as<uint32_t>();
		offColor = obj[KEY_OFF_COLOR].as<uint32_t>();
		onTime = obj[KEY_ON_TIME].as<uint32_t>();
		offTime = obj[KEY_OFF_TIME].as<uint32_t>();
		bounceTime = obj[KEY_BOUNCE_TIME].as<uint32_t>();
		fadeTime = obj[KEY_FADE_TIME].as<uint32_t>();

		fadeIncrement = obj[KEY_FADE_INCREMENT].as<uint8_t>();

		if( obj.containsKey(KEY_RELAY_NODES) )
		{
			relayNodeSize = 0;
			JsonArray& nestedArray = obj[KEY_RELAY_NODES].asArray();
			for(uint8_t i=0; i<nestedArray.size(); i++ )
			{
				relayNodes[i] = nestedArray[i].as<uint8_t>();
				relayNodeSize += 1;
			}
		}

		status = true;

	}

	return status;
} // end parse

uint8_t Command::buildCommand(uint8_t *buffer)
{
	uint8_t status = false;

	StaticJsonBuffer<CMD_BUFFER_SIZE> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	root[KEY_CMD] = command;
	root[KEY_UNIQUE_ID] = uniqueId;
	root[KEY_NODE_ID] = nodeId;
	root[KEY_NOTIFY_ON_COMPLETE] = notifyOnComplete;
	root[KEY_RELAY] = relay;
	if( relay == true )
	{
		JsonArray&  nestedArray  = root.createNestedArray(KEY_RELAY_NODES);
		for(uint8_t i = 0; i<relayNodeSize; i++)
		{
			nestedArray.add(relayNodes[i]);
		}
	}
	root[KEY_SHOW] = show;
	root[KEY_FPS] = framesPerSecond;
	root[KEY_UPDATE_TIME] = updateTime;
	root[KEY_INTENSITY] = intensity;
	root[KEY_INDEX] = index;
	root[KEY_PATTERN] = pattern;
	root[KEY_PATTERN_LENGTH] = patternLength;
	root[KEY_DURATION] = duration;
	root[KEY_REPEAT] = repeat;
	root[KEY_DIRECTION] = direction;
	root[KEY_FADE_BY] = fadeBy;
	root[KEY_PROBABILITY] = probability;
	root[KEY_CLEAR_AFTER] = clearAfter;
	root[KEY_CLEAR_END] = clearEnd;
	root[KEY_ON_COLOR] = onColor;
	root[KEY_OFF_COLOR] = offColor;
	root[KEY_ON_TIME] = onTime;
	root[KEY_OFF_TIME] = offTime;
	root[KEY_BOUNCE_TIME] = bounceTime;
	root[KEY_FADE_TIME] = fadeTime;
	root[KEY_FADE_INCREMENT] = fadeIncrement;

	root[KEY_REPEAT] = repeat;
	root[KEY_DURATION] = duration;

	root.printTo((char *)buffer, CMD_BUFFER_SIZE);

	status = true;

	return status;
}

uint8_t Command::buildResponse(uint8_t *buffer)
{
	uint8_t status = false;

	StaticJsonBuffer<CMD_BUFFER_SIZE> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	// Our command is "complete"
	root[KEY_CMD] = CMD_COMPLETE;

	// Who is complete
	root[KEY_NODE_ID] = getNodeId();

	// What unique id of command that was complete
	root[KEY_UNIQUE_ID] = getUniqueId();

	// Status of command (SUCCESS/FAIL)
	root[KEY_STATUS] = STATUS_SUCCESS;

	root.printTo((char *)buffer, CMD_BUFFER_SIZE);

	status = true;

	return status;
}

void Command::shiftRelayNodes()
{
	// Shift array one node to the left
	for(uint8_t i=0; i<relayNodeSize; i++)
	{
		relayNodes[i] = relayNodes[i+1];
	}
	relayNodes[relayNodeSize-1] = 0; // zero last node
	relayNodeSize -= 1;

}

void Command::dump()
{
#ifdef __DEBUG
	Serial.print("command: ");
	Serial.print( command, HEX );
	printf(", uid: %uld", uniqueId);
	Serial.print(", nodeId: ");
	Serial.print( nodeId, HEX );
	Serial.print(", notifyOncomplete: ");
	Serial.print( notifyOnComplete );
	Serial.print(", relay: ");
	Serial.print( relay );
	Serial.print(", relayNodes: {");
	if( relayNodeSize == 0 )
	{
		Serial.print("}, ");
	}
	for(uint8_t i=0; i<relayNodeSize; i++)
	{
		Serial.print( relayNodes[i]);
		if( i == (relayNodeSize-1) )
		{
			Serial.print("}, ");
			break;
		}
		else
		{
			Serial.print(",");
		}
	}
	Serial.print(", show: ");
	Serial.print( show );
	Serial.print(", fps: ");
	Serial.print( framesPerSecond );
	Serial.print(", updateTime:");
	Serial.print( updateTime );
	Serial.print(", intensity:");
	Serial.print( intensity );
	Serial.print(", index: ");
	Serial.print( index );
	Serial.print(", pattern: ");
	Serial.print( pattern, HEX );
	Serial.print(", patternLength: ");
	Serial.print( patternLength );
	Serial.print(", duration: ");
	Serial.print( duration );
	Serial.print(", repeat: ");
	Serial.print( repeat );
	Serial.print(", direction: ");
	Serial.print( direction );
	Serial.print(", fadeBy: ");
	Serial.print( fadeBy );
	Serial.print(", probability: ");
	Serial.print( probability );
	Serial.print(", clearAfter: ");
	Serial.print( clearAfter );
	Serial.print(", clearEnd: ");
	Serial.print( clearEnd );
	Serial.print(", onColor: ");
	Serial.print( onColor, HEX );
	Serial.print(", offColor: ");
	Serial.print( offColor, HEX );
	Serial.print(", onTime: ");
	Serial.print( onTime );
	Serial.print(", offTime: ");
	Serial.print( offTime );
	Serial.print(", bounceTime: ");
	Serial.print( bounceTime );
	Serial.print(", fadeTime: ");
	Serial.print( fadeTime );
	Serial.print(", fadeIncrement: ");
	Serial.println( fadeIncrement );

#endif
}

uint8_t Command::getCommand() const
{
	return command;
}

void Command::setCommand(uint8_t command)
{
	this->command = command;
}

uint32_t Command::getUniqueId() const
{
	return uniqueId;
}

void Command::setUniqueId(uint32_t uniqueId)
{
	this->uniqueId = uniqueId;
}

uint8_t Command::getNodeId() const
{
	return nodeId;
}

void Command::setNodeId(uint8_t nodeId)
{
	this->nodeId = nodeId;
}

uint8_t Command::getShow() const
{
	return show;
}

void Command::setShow(uint8_t show)
{
	this->show = show;
}


uint8_t Command::getIndex() const
{
	return index;
}

void Command::setIndex(uint8_t index)
{
	this->index = index;
}


uint32_t Command::getBounceTime() const
{
	return bounceTime;
}

void Command::setBounceTime(uint32_t bounceTime)
{
	this->bounceTime = bounceTime;
}

uint8_t Command::getClearAfter() const
{
	return clearAfter;
}

void Command::setClearAfter(uint8_t clearAfter)
{
	this->clearAfter = clearAfter;
}

uint8_t Command::getClearEnd() const
{
	return clearEnd;
}

void Command::setClearEnd(uint8_t clearEnd)
{
	this->clearEnd = clearEnd;
}

uint8_t Command::getDirection() const
{
	return direction;
}

void Command::setDirection(uint8_t direction)
{
	this->direction = direction;
}

uint32_t Command::getDuration() const
{
	return duration;
}

void Command::setDuration(uint32_t duration)
{
	this->duration = duration;
}

uint8_t Command::getFadeIncrement() const
{
	return fadeIncrement;
}

void Command::setFadeIncrement(uint8_t fadeIncrement)
{
	this->fadeIncrement = fadeIncrement;
}

uint32_t Command::getFadeTime() const
{
	return fadeTime;
}

void Command::setFadeTime(uint32_t fadeTime)
{
	this->fadeTime = fadeTime;
}

uint8_t Command::getFramesPerSecond() const
{
	return framesPerSecond;
}

void Command::setFramesPerSecond(uint8_t framesPerSecond)
{
	this->framesPerSecond = framesPerSecond;
}

uint8_t Command::getIntensity() const
{
	return intensity;
}

void Command::setIntensity(uint8_t i)
{
	this->intensity = i;
}

uint8_t Command::getFadeBy() const
{
	return fadeBy;
}

void Command::setFadeBy(uint8_t fadeBy)
{
	this->fadeBy = fadeBy;
}

CRGB Command::getOffColor() const
{
	return offColor;
}

void Command::setOffColor(uint32_t offColor)
{
	this->offColor = offColor;
}

uint32_t Command::getOffTime() const
{
	return offTime;
}

void Command::setOffTime(uint32_t offTime)
{
	this->offTime = offTime;
}

CRGB Command::getOnColor() const
{
	return onColor;
}

void Command::setOnColor(uint32_t onColor)
{
	this->onColor = onColor;
}

uint32_t Command::getOnTime() const
{
	return onTime;
}

void Command::setOnTime(uint32_t onTime)
{
	this->onTime = onTime;
}

uint8_t Command::getPattern() const
{
	return pattern;
}

void Command::setPattern(uint8_t pattern)
{
	this->pattern = pattern;
}

uint8_t Command::getPatternLength() const
{
	return patternLength;
}

void Command::setPatternLength(uint8_t patternLength)
{
	this->patternLength = patternLength;
}



uint8_t Command::getProbability() const
{
	return probability;
}

void Command::setProbability(uint8_t probability)
{
	this->probability = probability;
}

uint16_t Command::getRepeat() const
{
	return repeat;
}

void Command::setRepeat(uint16_t repeat)
{
	this->repeat = repeat;
}

uint8_t Command::getUpdateTime() const
{
	return updateTime;
}

void Command::setUpdateTime(uint8_t updateTime)
{
	this->updateTime = updateTime;
}

uint8_t Command::getNotifyOnComplete() const
{
	return notifyOnComplete;
}

void Command::setNotifyOnComplete(uint8_t notifyOnComplete)
{
	this->notifyOnComplete = notifyOnComplete;
}

uint8_t Command::getRelay() const
{
	return relay;
}

void Command::setRelay(uint8_t relay)
{
	this->relay = relay;
}


uint8_t* Command::getRelayNodes()
{
	return relayNodes;
}

void Command::setRelayNode(uint8_t index, uint8_t value)
{
	if( index < MAX_RELAY_NODES )
	{
		relayNodes[index] = value;
	}
}

uint8_t Command::getRelayNodeSize()
{
	return relayNodeSize;
}

