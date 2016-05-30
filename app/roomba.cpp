/*
 * roomba.cpp
 *
 *  Created on: 17 may 2016
 *      Author: Linus
 */

#include <roomba.h>


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



roomba::roomba(uint8_t wakepin)
{
	m_wakepin = wakepin;
	pinMode(m_wakepin, OUTPUT);
	_timerExpired = false;
}

void roomba::start(int processPeriod)
{
	_timer.initializeMs(processPeriod , TimerDelegate(&roomba::Process,this)).start();
}

void roomba::Process(void)
{

}


