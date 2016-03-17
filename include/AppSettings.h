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
	String ssid;
	String password;

	bool dhcp = true;

	IPAddress ip;
	IPAddress netmask;
	IPAddress gateway;

	String mqtt_server = "192.168.1.66";
	String mqtt_user = "user";
	String mqtt_password = "password";
	uint32 mqtt_period = 1800;
	uint32 mqtt_port = 1883;

	String mqtt_relayName = "Fonsterlampa";
	String mqtt_ledName = "IndicatorLED";

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
			mqtt_relayName = mqtt["relayName"].asString();
			mqtt_ledName = mqtt["ledName"].asString();

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

		mqtt["relayName"] = mqtt_relayName.c_str();
		mqtt["ledName"] = mqtt_ledName.c_str();

		JsonObject& ota = jsonBuffer.createObject();
		root["ota"] = ota;
		ota["rom0"] = ota_ROM_0.c_str();
		ota["rom1"] = ota_ROM_1.c_str();
		ota["spiffs"] = ota_SPIFFS.c_str();

		//TODO: add direct file stream writing
		String rootString;
		root.printTo(rootString);
		fileSetContent(APP_SETTINGS_FILE, rootString);
	}

	bool exist() { return fileExist(APP_SETTINGS_FILE); }
};

static ApplicationSettingsStorage AppSettings;

#endif /* INCLUDE_APPSETTINGS_H_ */
