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
	sint16 distance;
	sint16 angle;
	uint8 charge_state;
	uint16 voltage;
	sint16 current;
	sint8 temperature;
	uint16 charge;
	uint16 capacity;
} Roomba_Sensor_Data_t;


class roomba {
  public:
	roomba(uint8_t wakepin);
	void Process(void);
	void start(int processPeriod);
  private:
	uint8_t m_wakepin;
	Roomba_Sensor_Data_t	sensordata;
	bool _timerExpired;
	Timer _timer;
};

#endif /* APP_ROOMBA_H_ */
