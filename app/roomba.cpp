/*
 * roomba.cpp
 *
 *  Created on: 17 may 2016
 *      Author: Linus
 */

#include <roomba.h>



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


