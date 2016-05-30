/*
 * roomba.h
 *
 *  Created on: 17 may 2016
 *      Author: Linus
 */

#ifndef APP_ROOMBA_H_
#define APP_ROOMBA_H_

#include <config.h>
#include <stdlib.h>
#include <SmingCore/SmingCore.h>
/*
Detailed specification of the protocol can be found here:
https://cdn-shop.adafruit.com/datasheets/create_2_Open_Interface_Spec.pdf
http://www.robotappstore.com/files/KB/Roomba/Create%20Open%20Interface_v2.pdf
*/
#define ROOMBA_CMD_NONE			  0u
#define ROOMBA_CMD_START		128u
#define ROOMBA_CMD_BAUD			129u
#define ROOMBA_CMD_CONTROL		130u
#define ROOMBA_CMD_SAFE			131u
#define ROOMBA_CMD_FULL			132u
#define ROOMBA_CMD_POWER		133u
#define ROOMBA_CMD_SPOT			134u
#define ROOMBA_CMD_CLEAN		135u
#define ROOMBA_CMD_MAX			136u
#define ROOMBA_CMD_DRIVE		137u
#define ROOMBA_CMD_MOTORS		138u
#define ROOMBA_CMD_LEDS			139u
#define ROOMBA_CMD_SONG			140u
#define ROOMBA_CMD_PLAY			141u
#define ROOMBA_CMD_SENSORS		142u
#define ROOMBA_CMD_SEEK_DOCK	143u

/* Maybe these work to: */
#define ROOMBA_CMD_RESET		7u
#define ROOMBA_CMD_STOP			173u
#define ROOMBA_CMD_SCHEDULE		162u
#define ROOMBA_CMD_SET_TIME		163u


#define ROOMBA_BAUD_300			0u
#define ROOMBA_BAUD_600			1u
#define ROOMBA_BAUD_1200		2u
#define ROOMBA_BAUD_2400		3u
#define ROOMBA_BAUD_4800		4u
#define ROOMBA_BAUD_9600		5u
#define ROOMBA_BAUD_14400		6u
#define ROOMBA_BAUD_19200		7u
#define ROOMBA_BAUD_28800		8u
#define ROOMBA_BAUD_38400		9u
#define ROOMBA_BAUD_57600		10u
#define ROOMBA_BAUD_115200		11u


#define CHARGE_STATE_NOT_CHARGEING		0
#define CHARGE_STATE_CHARGING_RECOVERY	1
#define CHARGE_STATE_CHARGING			2
#define CHARGE_STATE_CHARGING_TRICKLE	3
#define CHARGE_STATE_WAITING			4
#define CHARGE_STATE_CHARGING_ERROR		5


typedef struct {
	uint8	not_used: 3;
	uint8	wheel_caster: 1;
	uint8	wheel_left: 1;
	uint8	wheel_right: 1;
	uint8	bump_left: 1;
	uint8	bump_right: 1;
} bumps_t;

typedef struct {
	uint8	not_used: 3;
	uint8	drive_left: 1;
	uint8	drive_right: 1;
	uint8	main_brush: 1;
	uint8	vacuum: 1;
	uint8	side_brush: 1;
} ovrcur_t;

typedef struct {
	uint8	not_used: 4;
	uint8	power: 1;
	uint8	spot: 1;
	uint8	clean: 1;
	uint8	max: 1;
} btn_t;

/* Roomba data structure */

typedef union
{
    /* MISRA 2004 Rule 5.7 VIOLATION: Refer to REF1 above                     */
    uint8 bytes[26];
    /* MISRA 2004 Rule 6.4 VIOLATIONS: Refer to REF2 above                    */
    typedef struct {
    	bumps_t bumps;
    	uint8 wall;
    	uint8 cliff_left;
    	uint8 cliff_front_left;
    	uint8 cliff_front_right;
    	uint8 cliff_right;
    	uint8 virtual_wall;
    	ovrcur_t overcurrent;
    	uint8 dirt_left;
    	uint8 dirt_right;
    	uint8 remote;
    	btn_t buttons;
    	sint16 distance __attribute__((__packed__));
    	sint16 angle __attribute__((__packed__));
    	uint8 charge_state;
    	uint16 voltage __attribute__((__packed__));
    	sint16 current __attribute__((__packed__));
    	sint8 temperature;
    	uint16 charge __attribute__((__packed__));
    	uint16 capacity __attribute__((__packed__));
    } values;
}Roomba_Sensor_Data_t;


class roomba {
  public:
	roomba(uint8_t wakepin);
	void Process(void);
	void start(int processPeriod);
  private:
	void serialCallBack(Stream& stream, char arrivedChar, unsigned short availableCharsCount);
	uint8_t m_wakepin;
	Roomba_Sensor_Data_t	sensordata;
	bool _timerExpired;
	Timer _timer;
	uint8_t buffer[64];
	uint8_t bufferposition;
	uint8_t expectedResponse;
};

#endif /* APP_ROOMBA_H_ */
