/*
 * roomba.cpp
 *
 *  Created on: 17 may 2016
 *      Author: Linus
 */

#include <roomba.h>
#include <SmingCore/SmingCore.h>

void roomba::serialCallBack(Stream& stream, char arrivedChar, unsigned short availableCharsCount) {
	switch (expectedResponse)
	{
	case ROOMBA_CMD_SENSORS:
		if (availableCharsCount == 26)
		{
			for (int i = 0; i < availableCharsCount; i++) {
				sensordata.bytes[i] = stream.read();
			}
			expectedResponse = ROOMBA_CMD_NONE;
		}
		break;
	default:
		//ignore this data
		break;
	}
}

void roomba::getSensorDataAsJson(JsonObject& rmb)
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
	rmb["distance"] = sensordata.values.distance;
	rmb["angle"] = sensordata.values.angle;
	rmb["charge_state"] = sensordata.values.charge_state;
	rmb["voltage"] = sensordata.values.voltage;
	rmb["current"] = sensordata.values.current;
	rmb["temperature"] = sensordata.values.temperature;
	rmb["charge"] = sensordata.values.charge;
	rmb["capacity"] = sensordata.values.capacity;
	//rmb.printTo(data);
	//return data;
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

void roomba::requestSensorData(uint8 sensorGroup)
{
	uint8 i = 0;
	char buffer[2];
	buffer[0]=142;
	buffer[1]=sensorGroup;
	if (sensorGroup == 0)
	{
		expectedResponse = ROOMBA_CMD_SENSORS;
	}
	while (i < 2)
	{
		Serial.print(buffer[i]);
		i++;
	}
}
bool roomba::isConnected()
{
	return connected;
}

void roomba::_connect()
{
	sendCommand(ROOMBA_CMD_START);
	digitalWrite(m_wakepin, LOW);
	this->requestSensorData(0);
	connected = true;
}

void roomba::connect()
{
	statetimer.initializeMs(300 , TimerDelegate(&roomba::Process,this)).startOnce();
	digitalWrite(m_wakepin, HIGH);
}

void roomba::disconnect()
{
	connected = false;
	digitalWrite(m_wakepin, LOW);
	sendCommand(ROOMBA_CMD_STOP);
}

roomba::roomba(uint8_t wakepin)
{
	m_wakepin = wakepin;
	pinMode(m_wakepin, OUTPUT);
	_timerExpired = false;
	expectedResponse = ROOMBA_CMD_NONE;
}

void roomba::start(int processPeriod)
{
	_timer.initializeMs(processPeriod , TimerDelegate(&roomba::Process,this)).start();
}

void roomba::Process(void)
{
	if (connected)
	{
		this->requestSensorData(0u);
	}
}


