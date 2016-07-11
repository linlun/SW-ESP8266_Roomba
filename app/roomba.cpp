/*
 * roomba.cpp
 *
 *  Created on: 17 may 2016
 *      Author: Linus
 */

#include <roomba.h>
#include <SmingCore.h>
/*

Sensor packet:
Id 0:
000 000 000 001 001 000 000 000 000 000 000 000 000 000 000 000 004 061 136 255 123 025 009 254 010 015

Id 6:
000 000 000 001 001 000 000 000 000 000 000 000 000 000 000 000 004 061 109 255 123 025 009 247 010 015 000 002 011 099 000 023 000 000 011 124 000 000 000 000 001 000 000 000 000 000 000 000 000 000 000 000

*/
void roomba::serialCallBack(Stream& stream, char arrivedChar, unsigned short availableCharsCount) {
	switch (expectedResponse)
	{
	case ROOMBA_CMD_SENSORS:
		if (availableCharsCount == 26)
		{
			for (int i = 0; i < availableCharsCount; i++) {
				sensordata.bytes[i] = stream.read();
			}
			//Switch bytes in 2 byte variables
			uint8 tmp;
			uint8 i=12;
			tmp = sensordata.bytes[i];
			sensordata.bytes[i] = sensordata.bytes[i+1];
			sensordata.bytes[i+1] = tmp;
			i=14;
			tmp = sensordata.bytes[i];
			sensordata.bytes[i] = sensordata.bytes[i+1];
			sensordata.bytes[i+1] = tmp;
			i=17;
			tmp = sensordata.bytes[i];
			sensordata.bytes[i] = sensordata.bytes[i+1];
			sensordata.bytes[i+1] = tmp;
			i=19;
			tmp = sensordata.bytes[i];
			sensordata.bytes[i] = sensordata.bytes[i+1];
			sensordata.bytes[i+1] = tmp;
			i=22;
			tmp = sensordata.bytes[i];
			sensordata.bytes[i] = sensordata.bytes[i+1];
			sensordata.bytes[i+1] = tmp;
			i=24;
			tmp = sensordata.bytes[i];
			sensordata.bytes[i] = sensordata.bytes[i+1];
			sensordata.bytes[i+1] = tmp;

			distance -= sensordata.values.distance;
			angle += sensordata.values.angle;
			expectedResponse = ROOMBA_CMD_NONE;
			//statetimer.stop();
			commandTimeout = 0;
			_newDataAvailable = true;
			if (faultcounter)
			{
				faultcounter--;
			}
			//sendCommand(ROOMBA_CMD_START);
		}
		break;
	default:
		//ignore this data
		for (int i = 0; i < availableCharsCount; i++) {
			(void) stream.read();
		}
		break;
	}
}

bool roomba::getSensorDataAsJson(JsonObject& rmb)
{
	if (connected)
	{
		//String data;
		//DynamicJsonBuffer jsonBuffer;
		//JsonObject& rmb = jsonBuffer.createObject();
		rmb["bumps"] = sensordata.values.bumps.b;
		rmb["wall"] = sensordata.values.wall;
		rmb["cliff_left"] = sensordata.values.cliff_left;
		rmb["cliff_front_left"] = sensordata.values.cliff_front_left;
		rmb["cliff_front_right"] = sensordata.values.cliff_front_right;
		rmb["cliff_right"] = sensordata.values.cliff_right;
		rmb["virtual_wall"] = sensordata.values.virtual_wall;
		rmb["overcurrent"] = sensordata.values.overcurrent.b;
		rmb["dirt_left"] = sensordata.values.dirt_left;
		rmb["dirt_right"] = sensordata.values.dirt_right;
		rmb["remote"] = sensordata.values.remote;
		rmb["buttons"] = sensordata.values.buttons.b;
		rmb["distance"] = distance;
		rmb["angle"] = angle;
		rmb["charge_state"] = sensordata.values.charge_state;
		rmb["voltage"] = sensordata.values.voltage;
		rmb["current"] = sensordata.values.current;
		rmb["temperature"] = sensordata.values.temperature;
		rmb["charge"] = sensordata.values.charge;
		rmb["capacity"] = sensordata.values.capacity;
		//rmb.printTo(data);
		return true;
	}
	return false;
}


void roomba::schedule(ApplicationSettingsStorage appsettings)
{
	uint8 i = 0;
	char buffer[16];
	char days = 0;
	if (appsettings.rmb_sch_sunday) { days |= 0x01; }
	if (appsettings.rmb_sch_monday) { days |= 0x02; }
	if (appsettings.rmb_sch_tuesday) { days |= 0x04; }
	if (appsettings.rmb_sch_wednesday) { days |= 0x08; }
	if (appsettings.rmb_sch_thursday) { days |= 0x10; }
	if (appsettings.rmb_sch_friday) { days |= 0x20; }
	if (appsettings.rmb_sch_saturday) { days |= 0x40; }
	if (appsettings.rmb_sch_sundayHour > 23) {appsettings.rmb_sch_sundayHour = 0;}
	if (appsettings.rmb_sch_mondayHour > 23) {appsettings.rmb_sch_mondayHour = 0;}
	if (appsettings.rmb_sch_tuesdayHour > 23) {appsettings.rmb_sch_tuesdayHour = 0;}
	if (appsettings.rmb_sch_wednesdayHour > 23) {appsettings.rmb_sch_wednesdayHour = 0;}
	if (appsettings.rmb_sch_thursdayHour > 23) {appsettings.rmb_sch_thursdayHour = 0;}
	if (appsettings.rmb_sch_fridayHour > 23) {appsettings.rmb_sch_fridayHour = 0;}
	if (appsettings.rmb_sch_saturdayHour > 23) {appsettings.rmb_sch_saturdayHour = 0;}
	buffer[i++] = 167;
	buffer[i++] = days;
	buffer[i++] = appsettings.rmb_sch_sundayHour;
	buffer[i++] = appsettings.rmb_sch_sundayMinute;
	buffer[i++] = appsettings.rmb_sch_mondayHour;
	buffer[i++] = appsettings.rmb_sch_mondayMinute;
	buffer[i++] = appsettings.rmb_sch_tuesdayHour;
	buffer[i++] = appsettings.rmb_sch_tuesdayMinute;
	buffer[i++] = appsettings.rmb_sch_wednesdayHour;
	buffer[i++] = appsettings.rmb_sch_wednesdayMinute;
	buffer[i++] = appsettings.rmb_sch_thursdayHour;
	buffer[i++] = appsettings.rmb_sch_thursdayMinute;
	buffer[i++] = appsettings.rmb_sch_fridayHour;
	buffer[i++] = appsettings.rmb_sch_fridayMinute;
	buffer[i++] = appsettings.rmb_sch_saturdayHour;
	buffer[i++] = appsettings.rmb_sch_saturdayMinute;
	i=0;
	while (i < 16)
	{
		Serial.print(buffer[i]);
		i++;
	}
}

void roomba::sendCommand(uint8 cmd)
{
	uint8 i = 0;
	char buffer[1];
	buffer[0]=cmd;
	while (i < 1)
	{
		Serial.print(buffer[i]);
		i++;
	}
}

void roomba::setTime()
{
	uint8 i = 0;
	char buffer[4];
	DateTime time = SystemClock.now();
	buffer[0]=168;
	buffer[1]=time.DayofWeek; //Day
	buffer[2]=time.Hour; //Hour
	buffer[3]=time.Minute; //Minute
	while (i < 4)
	{
		Serial.print(buffer[i]);
		i++;
	}
}
void roomba::_requestTimeout()
{
	expectedResponse = ROOMBA_CMD_NONE;
	//sendCommand(faultcounter);
	faultcounter+=10;
	//mqtt->publish("roomba/debug", "request timeout");
}

void roomba::requestSensorData(uint8 sensorGroup)
{
	uint8 i = 0;
	char buffer[2];
	if (expectedResponse == ROOMBA_CMD_NONE)
	{
		//Serial.println("Sending sensor data request");
		buffer[0]=142;
		buffer[1]=sensorGroup;
		if (sensorGroup == 0)
		{
			expectedResponse = ROOMBA_CMD_SENSORS;
		}
		while(Serial.available())
		{
			(void)Serial.read();
		}
		commandTimeout = 5;
		//statetimer.initializeMs(500 , TimerDelegate(&roomba::_requestTimeout,this)).startOnce();
		while (i < 2)
		{
			Serial.print(buffer[i]);
			i++;
		}
	}
}
bool roomba::isConnected()
{
	return connected;
}

void roomba::disconnect()
{
	connected = false;
	digitalWrite(m_wakepin, LOW);
	//sendCommand(ROOMBA_CMD_STOP);
	sendCommand(ROOMBA_CMD_POWER);
}

roomba::roomba(uint8_t wakepin, MqttClient* mqtt)
{
	this->mqtt = mqtt;
	connected = false;
	m_wakepin = wakepin;
	digitalWrite(m_wakepin, LOW);
	pinMode(m_wakepin, OUTPUT);
	digitalWrite(m_wakepin, LOW);
	_timerExpired = false;
	expectedResponse = ROOMBA_CMD_NONE;
}

void roomba::start(int processPeriod)
{
	_state = Roomba_Init;
	Serial.setCallback(StreamDataReceivedDelegate(&roomba::serialCallBack,this));
	Serial.begin(19200);
	_timer.initializeMs(processPeriod , TimerDelegate(&roomba::Process,this)).start();
}

void roomba::requestState(RoombaState newState)
{
	switch (_state)
	{
		case Roomba_Off:
		case Roomba_On:
		case Roomba_Dock:
		case Roomba_Clean:
			_requestedstate = newState;
			break;
		default:
			_requestedstate = Roomba_None;
			break;
	}
}
RoombaState roomba::getState(void)
{
	return _state;
}

String roomba::getStateString(void)
{
	switch (_state)
	{
		case Roomba_Init:
			return "init";
		case Roomba_Off:
			return "off";
		case Roomba_On:
			return "on";
		case Roomba_Dock:
			return "dock";
		case Roomba_Clean:
			return "clean";
		case Roomba_Max:
			return "max";
		case Roomba_initWake:
			return "initWake";
		case Roomba_initWakeLow:
			return "initWakeLow";
		default:
			return "Off";
	}
}

void roomba::Process(void)
{
	static uint8 internalState = 0;
	if (commandTimeout)
	{
		commandTimeout--;
		if (commandTimeout == 0)
		{
			_requestTimeout();
		}
	}


	if (_requestedstate == Roomba_Reset)
	{
		sendCommand(ROOMBA_CMD_RESET);
		_requestedstate = Roomba_None;
		internalState = 0;
		_state = Roomba_On;
	}

	if (faultcounter >= 150)
	{
		//sendCommand(ROOMBA_CMD_STOP);
		connected = false;
		sendCommand(ROOMBA_CMD_POWER);
		//sendCommand(ROOMBA_CMD_START);
		//sendCommand(ROOMBA_CMD_START);
		internalState = 0;
		//mqtt->publish("roomba/debug", "fault");
	}
	switch (_state)
	{
		case Roomba_Init:
		{
			switch (internalState)
			{
			case 0:
				sendCommand(ROOMBA_CMD_START);
				internalState++;
				break;
			case 1:
				while(Serial.available())
				{
					(void)Serial.read();
				}
				requestSensorData(0u);
				_newDataAvailable=false;
				internalState++;
				break;
			case 2:
			case 3:
			case 4:
				internalState++;
				if (_newDataAvailable)
				{
					internalState = 0;
					_state = Roomba_On;
					//mqtt->publish("roomba/debug", "On from init");
					faultcounter = 0;
					connected = true;
				}
				break;
			case 5:
				//sendCommand(ROOMBA_CMD_STOP);
				connected = false;
				sendCommand(ROOMBA_CMD_POWER);
				//sendCommand(ROOMBA_CMD_START);
				//sendCommand(ROOMBA_CMD_START);
				//sendCommand(ROOMBA_CMD_START);
				internalState = 0;
				_state = Roomba_Off;
				_requestedstate = Roomba_None;
				//mqtt->publish("roomba/debug", "off from init");
				break;
			default:

				break;
			}
			break;
		}
		case Roomba_On:
		{
			switch (_requestedstate)
			{
			case Roomba_On:
				_requestedstate = Roomba_None;
				break;
			case Roomba_Off:
				//sendCommand(ROOMBA_CMD_STOP);
				connected = false;
				sendCommand(ROOMBA_CMD_POWER);
				//sendCommand(ROOMBA_CMD_START);
				internalState = 0;
				_requestedstate = Roomba_None;
				_state = Roomba_Off;
				//mqtt->publish("roomba/debug", "off from on");
				break;
			case Roomba_Clean:
				sendCommand(ROOMBA_CMD_CLEAN);
				internalState = 0;
				_requestedstate = Roomba_None;
				_state = Roomba_Clean;
				//mqtt->publish("roomba/debug", "clean from on");
				break;
			case Roomba_Max:
				sendCommand(ROOMBA_CMD_MAX);
				internalState = 0;
				_requestedstate = Roomba_None;
				_state = Roomba_Clean;
				//mqtt->publish("roomba/debug", "max from on");
				break;
			case Roomba_Dock:
				sendCommand(ROOMBA_CMD_SEEK_DOCK);
				internalState = 0;
				_requestedstate = Roomba_None;
				_state = Roomba_Dock;
				//mqtt->publish("roomba/debug", "dock from on");
				break;
			default:
				break;
			}
			break;
		}
		case Roomba_Off:
		{
			switch (_requestedstate)
			{
			case Roomba_Off:
				_requestedstate = Roomba_None;
				break;
			case Roomba_On:
			case Roomba_Clean:
			case Roomba_Max:
			case Roomba_Dock:
				_state = Roomba_initWake;
				//mqtt->publish("roomba/debug", "wake from off");
				break;
			default:
				break;
			}
			break;
		}
		case Roomba_Clean:
		{
			switch (_requestedstate)
			{
			case Roomba_Off:
				//sendCommand(ROOMBA_CMD_STOP);
				connected = false;
				sendCommand(ROOMBA_CMD_POWER);
				internalState = 0;
				_requestedstate = Roomba_None;
				_state = Roomba_Off;
				//mqtt->publish("roomba/debug", "off from clean");
				break;
			case Roomba_On:
				sendCommand(ROOMBA_CMD_CLEAN);
				internalState = 0;
				_requestedstate = Roomba_None;
				_state = Roomba_On;
				//mqtt->publish("roomba/debug", "on from clean");
				break;
			case Roomba_Max:
				sendCommand(ROOMBA_CMD_MAX);
				delay(10);
				sendCommand(ROOMBA_CMD_MAX);
				internalState = 0;
				_requestedstate = Roomba_None;
				_state = Roomba_Clean;
				//mqtt->publish("roomba/debug", "max from clean");
				break;
			case Roomba_Dock:
				sendCommand(ROOMBA_CMD_SEEK_DOCK);
				delay(10);
				sendCommand(ROOMBA_CMD_SEEK_DOCK);
				internalState = 0;
				_requestedstate = Roomba_None;
				_state = Roomba_Dock;
				//mqtt->publish("roomba/debug", "dock from clean");
				break;
			case Roomba_Clean:
				_requestedstate = Roomba_None;
				break;
			default:
				break;
			}
			break;
		}
		case Roomba_Dock:
		{
			switch (_requestedstate)
			{
			case Roomba_Off:
				//sendCommand(ROOMBA_CMD_STOP);
				connected = false;
				sendCommand(ROOMBA_CMD_POWER);
				internalState = 0;
				_requestedstate = Roomba_None;
				_state = Roomba_Off;
				//mqtt->publish("roomba/debug", "off from dock");
				break;
			case Roomba_On:
				sendCommand(ROOMBA_CMD_SEEK_DOCK);
				internalState = 0;
				_requestedstate = Roomba_None;
				_state = Roomba_On;
				//mqtt->publish("roomba/debug", "on from dock");
				break;
			case Roomba_Max:
				sendCommand(ROOMBA_CMD_MAX);
				delay(10);
				sendCommand(ROOMBA_CMD_MAX);
				internalState = 0;
				_requestedstate = Roomba_None;
				_state = Roomba_Clean;
				//mqtt->publish("roomba/debug", "max from dock");
				break;
			case Roomba_Clean:
				sendCommand(ROOMBA_CMD_CLEAN);
				delay(10);
				sendCommand(ROOMBA_CMD_CLEAN);
				internalState = 0;
				_requestedstate = Roomba_None;
				_state = Roomba_Clean;
				//mqtt->publish("roomba/debug", "clean from dock");
				break;
			case Roomba_Dock:
				_requestedstate = Roomba_None;
				break;
			default:
				break;
			}
			break;
		}
		case Roomba_initWake:
		{
			digitalWrite(m_wakepin, HIGH);
			_state = Roomba_initWakeLow;
			//mqtt->publish("roomba/debug", "wake low from init wake");
			break;
		}
		case Roomba_initWakeLow:
		{
			digitalWrite(m_wakepin, LOW);
			internalState = 0;
			_state = Roomba_Init;
			//mqtt->publish("roomba/debug", "init from low");
			break;
		}
		default:
		{
			_state = Roomba_Init;
			break;
		}
	}

	if (connected)
	{
		this->requestSensorData(0u);
	} else {
		distance = 0;
		angle = 0;
	}
}


