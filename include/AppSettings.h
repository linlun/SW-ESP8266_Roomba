/*
 * AppSettings.h
 *
 *  Created on: 13 мая 2015 г.
 *      Author: Anakod
 */

#include <SmingCore/SmingCore.h>

#ifndef INCLUDE_APPSETTINGS_H_
#define INCLUDE_APPSETTINGS_H_

#define APP_SETTINGS_FILE ".settings.conf" // leading point for security reasons :)

struct ApplicationSettingsStorage
{
	String ssid = WIFI_SSID;
	String password = WIFI_PWD;

	bool dhcp = true;

	IPAddress ip;
	IPAddress netmask;
	IPAddress gateway;

	String mqtt_server = "192.168.1.66";
	String mqtt_user = "user";
	String mqtt_password = "password";
	uint32 mqtt_period = 1800;
	uint32 mqtt_port = 1883;

	String mqtt_roombaName = "pumba";


	bool rmb_sch_monday = false;
	uint8 rmb_sch_mondayHour = 0;
	uint8 rmb_sch_mondayMinute = 0;
	bool rmb_sch_tuesday = false;
	uint8 rmb_sch_tuesdayHour = 0;
	uint8 rmb_sch_tuesdayMinute = 0;
	bool rmb_sch_wednesday = false;
	uint8 rmb_sch_wednesdayHour = 0;
	uint8 rmb_sch_wednesdayMinute = 0;
	bool rmb_sch_thursday = false;
	uint8 rmb_sch_thursdayHour = 0;
	uint8 rmb_sch_thursdayMinute = 0;
	bool rmb_sch_friday = false;
	uint8 rmb_sch_fridayHour = 0;
	uint8 rmb_sch_fridayMinute = 0;
	bool rmb_sch_saturday = false;
	uint8 rmb_sch_saturdayHour = 0;
	uint8 rmb_sch_saturdayMinute = 0;
	bool rmb_sch_sunday = false;
	uint8 rmb_sch_sundayHour = 0;
	uint8 rmb_sch_sundayMinute = 0;

	String ota_ROM_0 = "http://hok.famlundin.org:80/rom0.bin";
	String ota_ROM_1 = "http://hok.famlundin.org:80/rom1.bin";
	String ota_SPIFFS = "http://hok.famlundin.org:80/spiff_rom.bin";

	void load()
	{
		DynamicJsonBuffer jsonBuffer;
		if (exist())
		{
			int size = fileGetSize(APP_SETTINGS_FILE);
			char* jsonString = new char[size + 1];
			fileGetContent(APP_SETTINGS_FILE, jsonString, size + 1);
			JsonObject& root = jsonBuffer.parseObject(jsonString);

			JsonObject& network = root["network"];
			ssid = network["ssid"].asString();
			password = network["password"].asString();

			dhcp = network["dhcp"];

			ip = network["ip"].asString();
			netmask = network["netmask"].asString();
			gateway = network["gateway"].asString();

			JsonObject& mqtt = root["mqtt"];
			mqtt_user = mqtt["user"].asString();
			mqtt_password = mqtt["password"].asString();
			mqtt_server = mqtt["server"].asString();
			mqtt_port = mqtt["port"];
			mqtt_period = mqtt["period"];
			mqtt_roombaName = mqtt["roombaName"].asString();

			JsonObject& rmb_sch = root["rmb_sch"];
			rmb_sch_monday = rmb_sch["monday"];
			rmb_sch_mondayHour = rmb_sch["mondayHour"];
			rmb_sch_mondayMinute = rmb_sch["mondayMinute"];
			rmb_sch_tuesday = rmb_sch["tuesday"];
			rmb_sch_tuesdayHour = rmb_sch["tuesdayHour"];
			rmb_sch_tuesdayMinute = rmb_sch["tuesdayMinute"];
			rmb_sch_wednesday = rmb_sch["wednesday"];
			rmb_sch_wednesdayHour = rmb_sch["wednesdayHour"];
			rmb_sch_wednesdayMinute = rmb_sch["wednesdayMinute"];
			rmb_sch_thursday = rmb_sch["thursday"];
			rmb_sch_thursdayHour = rmb_sch["thursdayHour"];
			rmb_sch_thursdayMinute = rmb_sch["thursdayMinute"];
			rmb_sch_friday = rmb_sch["friday"];
			rmb_sch_fridayHour = rmb_sch["fridayHour"];
			rmb_sch_fridayMinute = rmb_sch["fridayMinute"];
			rmb_sch_saturday = rmb_sch["saturday"];
			rmb_sch_saturdayHour = rmb_sch["saturdayHour"];
			rmb_sch_saturdayMinute = rmb_sch["saturdayMinute"];
			rmb_sch_sunday = rmb_sch["sunday"];
			rmb_sch_sundayHour = rmb_sch["sundayHour"];
			rmb_sch_sundayMinute = rmb_sch["sundayMinute"];

			JsonObject& ota = root["ota"];
			ota_ROM_0 = ota["rom0"].asString();
			ota_ROM_0 = ota["rom1"].asString();
			ota_SPIFFS = ota["spiffs"].asString();

			delete[] jsonString;
		}
	}

	void save()
	{
		DynamicJsonBuffer jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();

		JsonObject& network = jsonBuffer.createObject();
		root["network"] = network;
		network["ssid"] = ssid.c_str();
		network["password"] = password.c_str();

		network["dhcp"] = dhcp;

		// Make copy by value for temporary string objects
		network["ip"] = ip.toString();
		network["netmask"] = netmask.toString();
		network["gateway"] = gateway.toString();


		JsonObject& mqtt = jsonBuffer.createObject();
		root["mqtt"] = mqtt;
		mqtt["user"] = mqtt_user.c_str();
		mqtt["password"] = mqtt_password.c_str();
		mqtt["server"] = mqtt_server.c_str();
		mqtt["port"] = mqtt_port;
		mqtt["period"] = mqtt_period;

		mqtt["roombaName"] = mqtt_roombaName.c_str();

		JsonObject& ota = jsonBuffer.createObject();
		root["ota"] = ota;
		ota["rom0"] = ota_ROM_0.c_str();
		ota["rom1"] = ota_ROM_1.c_str();
		ota["spiffs"] = ota_SPIFFS.c_str();


		JsonObject& rmb_sch = jsonBuffer.createObject();
		root["rmb_sch"] = rmb_sch;
		rmb_sch["monday"] = rmb_sch_monday;
		rmb_sch["mondayHour"] = rmb_sch_mondayHour;
		rmb_sch["mondayMinute"] = rmb_sch_mondayMinute;
		rmb_sch["tuesday"] = rmb_sch_tuesday;
		rmb_sch["tuesdayHour"] = rmb_sch_tuesdayHour;
		rmb_sch["tuesdayMinute"] = rmb_sch_tuesdayMinute;
		rmb_sch["wednesday"] = rmb_sch_wednesday;
		rmb_sch["wednesdayHour"] = rmb_sch_wednesdayHour;
		rmb_sch["wednesdayMinute"] = rmb_sch_wednesdayMinute;
		rmb_sch["thursday"] = rmb_sch_thursday;
		rmb_sch["thursdayHour"] = rmb_sch_thursdayHour;
		rmb_sch["thursdayMinute"] = rmb_sch_thursdayMinute;
		rmb_sch["friday"] = rmb_sch_friday;
		rmb_sch["fridayHour"] = rmb_sch_fridayHour;
		rmb_sch["fridayMinute"] = rmb_sch_fridayMinute;
		rmb_sch["saturday"] = rmb_sch_saturday;
		rmb_sch["saturdayHour"] = rmb_sch_saturdayHour;
		rmb_sch["saturdayMinute"] = rmb_sch_saturdayMinute;
		rmb_sch["sunday"] = rmb_sch_sunday;
		rmb_sch["sundayHour"] = rmb_sch_sundayHour;
		rmb_sch["sundayMinute"] = rmb_sch_sundayMinute;

		//TODO: add direct file stream writing
		String rootString;
		root.printTo(rootString);
		fileSetContent(APP_SETTINGS_FILE, rootString);
	}

	bool exist() { return fileExist(APP_SETTINGS_FILE); }
};

static ApplicationSettingsStorage AppSettings;

#endif /* INCLUDE_APPSETTINGS_H_ */
