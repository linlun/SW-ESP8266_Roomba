#include <user_config.h>
#include "SmingCore.h"
#include <AppSettings.h>
#include <config.h>
#include "roomba.h"
#include "NtpClientDelegateDemo.h"

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "blablabla" // Put you SSID and Password here
	#define WIFI_PWD "blablabla"
#endif

HttpServer server;
roomba roomba(PIN_ROOBA_WAKE);
BssList networks;
String network, password;
String mqtt_client;
rBootHttpUpdate* otaUpdater = 0;

ntpClientDemo *ntpDemo;
Timer printTimer;

// Forward declarations
void startMqttClient();
void onMessageReceived(String topic, String message);

Timer procTimer;
/*
void onPrintSystemTime() {
	Serial.print("Local Time    : ");
	Serial.println(SystemClock.getSystemTimeString());
	Serial.print("UTC Time: ");
	Serial.println(SystemClock.getSystemTimeString(eTZ_UTC));
}
*/

// MQTT client
// For quick check you can use: http://www.hivemq.com/demos/websocket-client/ (Connection= test.mosquitto.org:8080)
MqttClient *mqtt;


void OtaUpdate_CallBack(bool result) {
	
	Serial.println("In callback...");
	if(result == true) {
		// success
		uint8 slot;
		slot = rboot_get_current_rom();
		if (slot == 0) slot = 1; else slot = 0;
		// set to boot new rom and then reboot
		Serial.printf("Firmware updated, rebooting to rom %d...\r\n", slot);
		rboot_set_current_rom(slot);
		System.restart();
	} else {
		// fail
		Serial.println("Firmware update failed!");
	}
}

void OtaUpdate() {
	
	uint8 slot;
	rboot_config bootconf;
	
	Serial.println("Updating...");
	
	// need a clean object, otherwise if run before and failed will not run again
	if (otaUpdater) delete otaUpdater;
	otaUpdater = new rBootHttpUpdate();
	
	// select rom slot to flash
	bootconf = rboot_get_config();
	slot = bootconf.current_rom;
	if (slot == 0) slot = 1; else slot = 0;

#ifndef RBOOT_TWO_ROMS
	// flash rom to position indicated in the rBoot config rom table
	otaUpdater->addItem(bootconf.roms[slot], AppSettings.ota_ROM_0);
#else
	// flash appropriate rom
	if (slot == 0) {
		otaUpdater->addItem(bootconf.roms[slot],AppSettings.ota_ROM_0);
	} else {
		otaUpdater->addItem(bootconf.roms[slot], AppSettings.ota_ROM_1);
	}
#endif
	
#ifndef DISABLE_SPIFFS
	// use user supplied values (defaults for 4mb flash in makefile)
	if (slot == 0) {
		otaUpdater->addItem(RBOOT_SPIFFS_0, AppSettings.ota_SPIFFS);
	} else {
		otaUpdater->addItem(RBOOT_SPIFFS_1, AppSettings.ota_SPIFFS);
	}
#endif

	// request switch and reboot on success
	//otaUpdater->switchToRom(slot);
	// and/or set a callback (called on failure or success without switching requested)
	otaUpdater->setCallback(OtaUpdate_CallBack);

	// start update
	otaUpdater->start();
}

void Switch() {
	uint8 before, after;
	before = rboot_get_current_rom();
	if (before == 0) after = 1; else after = 0;
	Serial.printf("Swapping from rom %d to rom %d.\r\n", before, after);
	rboot_set_current_rom(after);
	Serial.println("Restarting...\r\n");
	System.restart();
}

void ShowInfo() {
    Serial.printf("\r\nSDK: v%s\r\n", system_get_sdk_version());
    Serial.printf("Free Heap: %d\r\n", system_get_free_heap_size());
    Serial.printf("CPU Frequency: %d MHz\r\n", system_get_cpu_freq());
    Serial.printf("System Chip ID: %x\r\n", system_get_chip_id());
    Serial.printf("SPI Flash ID: %x\r\n", spi_flash_get_id());
    //Serial.printf("SPI Flash Size: %d\r\n", (1 << ((spi_flash_get_id() >> 16) & 0xff)));
}

void serialCallBack(Stream& stream, char arrivedChar, unsigned short availableCharsCount) {
	if (arrivedChar == '\n') {
		char str[availableCharsCount];
		for (int i = 0; i < availableCharsCount; i++) {
			str[i] = stream.read();
			if (str[i] == '\r' || str[i] == '\n') {
				str[i] = '\0';
			}
		}
		
		if (!strcmp(str, "connect")) {
			// connect to wifi
			WifiStation.config(WIFI_SSID, WIFI_PWD);
			WifiStation.enable(true);
		} else if (!strcmp(str, "ip")) {
			Serial.printf("ip: %s mac: %s\r\n", WifiStation.getIP().toString().c_str(), WifiStation.getMAC().c_str());
		} else if (!strcmp(str, "ota")) {
			OtaUpdate();
		} else if (!strcmp(str, "switch")) {
			Switch();
		} else if (!strcmp(str, "restart")) {
			System.restart();
		} else if (!strcmp(str, "ls")) {
			Vector<String> files = fileList();
			Serial.printf("filecount %d\r\n", files.count());
			for (unsigned int i = 0; i < files.count(); i++) {
				Serial.println(files[i]);
			}
		} else if (!strcmp(str, "cat")) {
			Vector<String> files = fileList();
			if (files.count() > 0) {
				Serial.printf("dumping file %s:\r\n", files[0].c_str());
				//Serial.println(fileGetContent(files[0]));
				Serial.println(fileGetContent(".settings.conf"));

			} else {
				Serial.println("Empty spiffs!");
			}
		} else if (!strcmp(str, "info")) {
			ShowInfo();
		} else if (!strcmp(str, "help")) {
			Serial.println();
			Serial.println("available commands:");
			Serial.println("  help - display this message");
			Serial.println("  ip - show current ip address");
			Serial.println("  connect - connect to wifi");
			Serial.println("  restart - restart the esp8266");
			Serial.println("  switch - switch to the other rom and reboot");
			Serial.println("  ota - perform ota update, switch rom and reboot");
			Serial.println("  info - show esp8266 info");
			Serial.println("  sl - Sleep for 5 seconds");
#ifndef DISABLE_SPIFFS
			Serial.println("  ls - list files in spiffs");
			Serial.println("  cat - show first file in spiffs");
#endif
			Serial.println();
			//roomba.connect();
		} else if (!strcmp(str, "sl")) {
			Serial.println("Going to sleep");
			delay(500);
			system_deep_sleep(5000000);
			delay(500);
			Serial.println("Wakeing up");
		} else {
			Serial.println("unknown command");
		}
		roomba.setTime();
	}
}

// Check for MQTT Disconnection
void checkMQTTDisconnect(TcpClient& client, bool flag){

	// Called whenever MQTT connection is failed.
	if (flag == true)
		Serial.println("MQTT Broker Disconnected!!");
	else
		Serial.println("MQTT Broker Unreachable!!");

	// Restart connection attempt after few seconds
	procTimer.initializeMs(2 * 1000, startMqttClient).start(); // every 2 seconds
}

// Publish our message
void publishMessage()
{
	if (mqtt->getConnectionState() != eTCS_Connected)
		startMqttClient(); // Auto reconnect

	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	if (roomba.getSensorDataAsJson(json))
	{
		String rootString;
		json.printTo(rootString);
		mqtt->publish(mqtt_client + "/" +AppSettings.mqtt_roombaName + "/sensors", rootString); // or publishWithQoS
	}
	mqtt->publish(mqtt_client + "/" +AppSettings.mqtt_roombaName + "/status", roomba.getStateString()); // or publishWithQoS

}

// Callback for messages, arrived from MQTT server
void onMessageReceived(String topic, String message)
{
	if (topic.startsWith(mqtt_client + "/" +AppSettings.mqtt_roombaName + "/"))
	{
		if (topic.endsWith("status"))
		{
			if (message.equals("clean"))
			{
				roomba.requestState(Roomba_Clean);
			} else if (message.equals("dock"))
			{
				roomba.requestState(Roomba_Dock);
			} else if (message.equals("max"))
			{
				roomba.requestState(Roomba_Max);
			} else if (message.equals("off"))
			{
				roomba.requestState(Roomba_Off);
			} else if (message.equals("on"))
			{
				roomba.requestState(Roomba_On);
			}
		}
	}

	//Serial.print(topic);
	//Serial.print(":\r\n\t"); // Pretify alignment for printing
	//Serial.println(message);
}

// Run MQTT client

void startMqttClient()
{
	/*if (mqtt == null)
	{
		mqtt = new MqttClient(MQTT_HOST, MQTT_PORT, onMessageReceived);
	}
	*/
	procTimer.stop();
	if(!mqtt->setWill("last/will","The connection from this device is lost:(", 1, true)) {
		debugf("Unable to set the last will and testament. Most probably there is not enough memory on the device.");
	}
	mqtt_client = "esp8266_" + WifiStation.getMAC();
	mqtt->connect(mqtt_client, MQTT_USERNAME, MQTT_PWD);
	// Assign a disconnect callback function
	mqtt->setCompleteDelegate(checkMQTTDisconnect);
	mqtt->subscribe(mqtt_client + "/" +AppSettings.mqtt_roombaName + "/#");
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	//Serial.println("I'm CONNECTED");
	ntpDemo = new ntpClientDemo();
	// Run MQTT client
	startMqttClient();

	// Start publishing loop
	procTimer.initializeMs(5 * 1000, publishMessage).start(); // every 20 seconds
	//roomba.connect();
	roomba.start(100);
}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	Serial.println("I'm NOT CONNECTED. Need help :(");

	// .. some you code for device configuration ..
}

Timer connectionTimer;

void onIndex(HttpRequest &request, HttpResponse &response)
{
	TemplateFileStream *tmpl = new TemplateFileStream("index.html");
	auto &vars = tmpl->variables();
	response.sendTemplate(tmpl); // will be automatically deleted
}

void onIpConfig(HttpRequest &request, HttpResponse &response)
{
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		AppSettings.dhcp = request.getPostParameter("dhcp") == "1";
		AppSettings.ip = request.getPostParameter("ip");
		AppSettings.netmask = request.getPostParameter("netmask");
		AppSettings.gateway = request.getPostParameter("gateway");
		debugf("Updating IP settings: %d", AppSettings.ip.isNull());
		AppSettings.save();
	}

	TemplateFileStream *tmpl = new TemplateFileStream("settings.html");
	auto &vars = tmpl->variables();

	bool dhcp = WifiStation.isEnabledDHCP();
	vars["dhcpon"] = dhcp ? "checked='checked'" : "";
	vars["dhcpoff"] = !dhcp ? "checked='checked'" : "";

	if (!WifiStation.getIP().isNull())
	{
		vars["ip"] = WifiStation.getIP().toString();
		vars["netmask"] = WifiStation.getNetworkMask().toString();
		vars["gateway"] = WifiStation.getNetworkGateway().toString();
	}
	else
	{
		vars["ip"] = "192.168.1.77";
		vars["netmask"] = "255.255.255.0";
		vars["gateway"] = "192.168.1.1";
	}

	response.sendTemplate(tmpl); // will be automatically deleted
}

void onRoombaCtrl(HttpRequest &request, HttpResponse &response)
{
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		/*
		Serial.printf("Updating MQTT settings: %d", AppSettings.ip.isNull());
		roomba.schedule(AppSettings);
		AppSettings.save();
		*/
	}

	TemplateFileStream *tmpl = new TemplateFileStream("roomba_ctrl.html");
	auto &vars = tmpl->variables();
	/*
	vars["mondayon"] = AppSettings.rmb_sch_monday ? "checked='checked'" : "";
	vars["mondayHour"] = AppSettings.rmb_sch_mondayHour;
	*/
	response.sendTemplate(tmpl); // will be automatically deleted
}
void onRoombaScheduling(HttpRequest &request, HttpResponse &response)
{
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		AppSettings.rmb_sch_monday = request.getPostParameter("monday") == "1";
		AppSettings.rmb_sch_mondayHour = request.getPostParameter("mondayHour").toInt();
		AppSettings.rmb_sch_mondayMinute = request.getPostParameter("mondayMinute").toInt();
		AppSettings.rmb_sch_tuesday = request.getPostParameter("tuesday") == "1";
		AppSettings.rmb_sch_tuesdayHour = request.getPostParameter("tuesdayHour").toInt();
		AppSettings.rmb_sch_tuesdayMinute = request.getPostParameter("tuesdayMinute").toInt();
		AppSettings.rmb_sch_wednesday = request.getPostParameter("wednesday") == "1";
		AppSettings.rmb_sch_wednesdayHour = request.getPostParameter("wednesdayHour").toInt();
		AppSettings.rmb_sch_wednesdayMinute = request.getPostParameter("wednesdayMinute").toInt();
		AppSettings.rmb_sch_thursday = request.getPostParameter("thursday") == "1";
		AppSettings.rmb_sch_thursdayHour = request.getPostParameter("thursdayHour").toInt();
		AppSettings.rmb_sch_thursdayMinute = request.getPostParameter("thursdayMinute").toInt();
		AppSettings.rmb_sch_friday = request.getPostParameter("friday") == "1";
		AppSettings.rmb_sch_fridayHour = request.getPostParameter("fridayHour").toInt();
		AppSettings.rmb_sch_fridayMinute = request.getPostParameter("fridayMinute").toInt();
		AppSettings.rmb_sch_saturday = request.getPostParameter("saturday") == "1";
		AppSettings.rmb_sch_saturdayHour = request.getPostParameter("saturdayHour").toInt();
		AppSettings.rmb_sch_saturdayMinute = request.getPostParameter("saturdayMinute").toInt();
		AppSettings.rmb_sch_sunday = request.getPostParameter("sunday") == "1";
		AppSettings.rmb_sch_sundayHour = request.getPostParameter("sundayHour").toInt();
		AppSettings.rmb_sch_sundayMinute = request.getPostParameter("sundayMinute").toInt();
		Serial.printf("Updating MQTT settings: %d", AppSettings.ip.isNull());
		roomba.schedule(AppSettings);
		AppSettings.save();

	}

	TemplateFileStream *tmpl = new TemplateFileStream("roomba_scheduling.html");
	auto &vars = tmpl->variables();
	vars["mondayon"] = AppSettings.rmb_sch_monday ? "checked='checked'" : "";
	vars["mondayoff"] = !AppSettings.rmb_sch_monday ? "checked='checked'" : "";
	vars["mondayHour"] = AppSettings.rmb_sch_mondayHour;
	vars["mondayMinute"] = AppSettings.rmb_sch_mondayMinute;
	vars["tuesdayon"] = AppSettings.rmb_sch_tuesday ? "checked='checked'" : "";
	vars["tuesdayoff"] = !AppSettings.rmb_sch_tuesday ? "checked='checked'" : "";
	vars["tuesdayHour"] = AppSettings.rmb_sch_tuesdayHour;
	vars["tuesdayMinute"] = AppSettings.rmb_sch_tuesdayMinute;
	vars["wednesdayon"] = AppSettings.rmb_sch_wednesday ? "checked='checked'" : "";
	vars["wednesdayoff"] = !AppSettings.rmb_sch_wednesday ? "checked='checked'" : "";
	vars["wednesdayHour"] = AppSettings.rmb_sch_wednesdayHour;
	vars["wednesdayMinute"] = AppSettings.rmb_sch_wednesdayMinute;
	vars["thursdayon"] = AppSettings.rmb_sch_thursday ? "checked='checked'" : "";
	vars["thursdayoff"] = !AppSettings.rmb_sch_thursday ? "checked='checked'" : "";
	vars["thursdayHour"] = AppSettings.rmb_sch_thursdayHour;
	vars["thursdayMinute"] = AppSettings.rmb_sch_thursdayMinute;
	vars["fridayon"] = AppSettings.rmb_sch_friday ? "checked='checked'" : "";
	vars["fridayoff"] = !AppSettings.rmb_sch_friday ? "checked='checked'" : "";
	vars["fridayHour"] = AppSettings.rmb_sch_fridayHour;
	vars["fridayMinute"] = AppSettings.rmb_sch_fridayMinute;
	vars["saturdayon"] = AppSettings.rmb_sch_saturday ? "checked='checked'" : "";
	vars["saturdayoff"] = !AppSettings.rmb_sch_saturday ? "checked='checked'" : "";
	vars["saturdayHour"] = AppSettings.rmb_sch_saturdayHour;
	vars["saturdayMinute"] = AppSettings.rmb_sch_saturdayMinute;
	vars["sundayon"] = AppSettings.rmb_sch_sunday ? "checked='checked'" : "";
	vars["sundayoff"] = !AppSettings.rmb_sch_sunday ? "checked='checked'" : "";
	vars["sundayHour"] = AppSettings.rmb_sch_sundayHour;
	vars["sundayMinute"] = AppSettings.rmb_sch_sundayMinute;

	response.sendTemplate(tmpl); // will be automatically deleted
}


void onMqttConfig(HttpRequest &request, HttpResponse &response)
{
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		AppSettings.mqtt_password = request.getPostParameter("password");
		AppSettings.mqtt_user = request.getPostParameter("user");
		AppSettings.mqtt_server = request.getPostParameter("adr");
		AppSettings.mqtt_period = request.getPostParameter("period").toInt();
		AppSettings.mqtt_port = request.getPostParameter("port").toInt();
		AppSettings.mqtt_roombaName = request.getPostParameter("roombaName");
		//debugf("Updating MQTT settings: %d", AppSettings.ip.isNull());
		AppSettings.save();
	}

	TemplateFileStream *tmpl = new TemplateFileStream("mqttsettings.html");
	auto &vars = tmpl->variables();

	vars["user"] = AppSettings.mqtt_user;
	vars["password"] = AppSettings.mqtt_password;
	vars["period"] = AppSettings.mqtt_period;
	vars["port"] = AppSettings.mqtt_port;
	vars["adr"] = AppSettings.mqtt_server;
	vars["roombaName"] = AppSettings.mqtt_roombaName;

	response.sendTemplate(tmpl); // will be automatically deleted
}

void onOtaConfig(HttpRequest &request, HttpResponse &response)
{
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		AppSettings.ota_ROM_0 = request.getPostParameter("rom0");
		AppSettings.ota_SPIFFS = request.getPostParameter("spiffs");
		AppSettings.save();
	}

	TemplateFileStream *tmpl = new TemplateFileStream("otasettings.html");
	auto &vars = tmpl->variables();

	vars["rom0"] = AppSettings.ota_ROM_0;
	vars["spiffs"] = AppSettings.ota_SPIFFS;

	response.sendTemplate(tmpl); // will be automatically deleted
}

void onSensors(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	if (roomba.getSensorDataAsJson(json))
	{
		response.sendJsonObject(stream);
	}
}

void onFile(HttpRequest &request, HttpResponse &response)
{
	String file = request.getPath();
	if (file[0] == '/')
		file = file.substring(1);

	if (file[0] == '.')
		response.forbidden();
	else
	{
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void onAjaxNetworkList(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	json["status"] = (bool)true;

	bool connected = WifiStation.isConnected();
	json["connected"] = connected;
	if (connected)
	{
		// Copy full string to JSON buffer memory
		json["network"]= WifiStation.getSSID();
	}

	JsonArray& netlist = json.createNestedArray("available");
	for (int i = 0; i < networks.count(); i++)
	{
		if (networks[i].hidden) continue;
		JsonObject &item = netlist.createNestedObject();
		item["id"] = (int)networks[i].getHashId();
		// Copy full string to JSON buffer memory
		item["title"] = networks[i].ssid;
		item["signal"] = networks[i].rssi;
		item["encryption"] = networks[i].getAuthorizationMethodName();
	}

	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}
void onAjaxRunOta(HttpRequest &request, HttpResponse &response)
{
	OtaUpdate();
}
void makeConnection()
{
	WifiStation.enable(true);
	WifiStation.config(network, password);

	AppSettings.ssid = network;
	AppSettings.password = password;
	AppSettings.save();

	network = ""; // task completed
}

void onAjaxConnect(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	String curNet = request.getPostParameter("network");
	String curPass = request.getPostParameter("password");

	bool updating = curNet.length() > 0 && (WifiStation.getSSID() != curNet || WifiStation.getPassword() != curPass);
	bool connectingNow = WifiStation.getConnectionStatus() == eSCS_Connecting || network.length() > 0;

	if (updating && connectingNow)
	{
		debugf("wrong action: %s %s, (updating: %d, connectingNow: %d)", network.c_str(), password.c_str(), updating, connectingNow);
		json["status"] = (bool)false;
		json["connected"] = (bool)false;
	}
	else
	{
		json["status"] = (bool)true;
		if (updating)
		{
			network = curNet;
			password = curPass;
			debugf("CONNECT TO: %s %s", network.c_str(), password.c_str());
			json["connected"] = false;
			connectionTimer.initializeMs(1200, makeConnection).startOnce();
		}
		else
		{
			json["connected"] = WifiStation.isConnected();
			debugf("Network already selected. Current status: %s", WifiStation.getConnectionStatusName());
		}
	}

	if (!updating && !connectingNow && WifiStation.isConnectionFailed())
		json["error"] = WifiStation.getConnectionStatusName();

	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}

void startWebServer()
{
	server.listen(80);
	server.addPath("/", onIndex);
	server.addPath("/ipconfig", onIpConfig);
	server.addPath("/mqttconfig", onMqttConfig);
	server.addPath("/ota", onOtaConfig);
	server.addPath("/ajax/get-networks", onAjaxNetworkList);
	server.addPath("/ajax/run-ota", onAjaxRunOta);
	server.addPath("/ajax/connect", onAjaxConnect);
	server.addPath("/Roomba_Scheduling", onRoombaScheduling);
	server.addPath("/Roomba_Control", onRoombaCtrl);
	server.addPath("/sensors", onSensors);


	server.setDefaultHandler(onFile);
}


void networkScanCompleted(bool succeeded, BssList list)
{
	if (succeeded)
	{
		for (int i = 0; i < list.count(); i++)
			if (!list[i].hidden && list[i].ssid.length() > 0)
				networks.add(list[i]);
	}
	networks.sort([](const BssInfo& a, const BssInfo& b){ return b.rssi - a.rssi; } );
}


void init() {
	char hostnametest[]="TEST12";
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
		//Serial.print("wifi_station_get_hostname: ");
	  //Serial.println(wifi_station_get_hostname());
	//wifi_station_set_hostname(&hostnametest[0]);
	//Serial.print("wifi_station_get_hostname: ");
	//  Serial.println(wifi_station_get_hostname());
	//system_set_os_print(0);
	Serial.systemDebugOutput(false); // Debug output to serial
	//system_update_cpu_freq(SYS_CPU_160MHZ);
	pinMode(5, OUTPUT);
	digitalWrite(5,1);

	// mount spiffs
	int slot = rboot_get_current_rom();
#ifndef DISABLE_SPIFFS
	if (slot == 0) {
#ifdef RBOOT_SPIFFS_0
		debugf("trying to mount spiffs at %x, length %d", RBOOT_SPIFFS_0 , SPIFF_SIZE);
		spiffs_mount_manual(RBOOT_SPIFFS_0, SPIFF_SIZE);
#else
		debugf("trying to mount spiffs at %x, length %d", 0x100000, SPIFF_SIZE);
		spiffs_mount_manual(0x100000, SPIFF_SIZE);
#endif
	} else {
#ifdef RBOOT_SPIFFS_1
		debugf("trying to mount spiffs at %x, length %d", RBOOT_SPIFFS_1 , SPIFF_SIZE);
		spiffs_mount_manual(RBOOT_SPIFFS_1, SPIFF_SIZE);
#else
		debugf("trying to mount spiffs at %x, length %d", SPIFF_SIZE);
		spiffs_mount_manual(0x300000, SPIFF_SIZE);
#endif
	}

#else
	debugf("spiffs disabled");
#endif

	AppSettings.load();

	WifiAccessPoint.enable(false);
	// connect to wifi
	//WifiStation.hostname= "test12";
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	char *host = &AppSettings.mqtt_roombaName[0];
	wifi_station_set_hostname(host);
	WifiStation.enable(true);
	
	if (AppSettings.exist())
	{
		WifiStation.config(AppSettings.ssid, AppSettings.password);
		//wifi_station_set_hostname(&hostnametest[0]);
		if (!AppSettings.dhcp && !AppSettings.ip.isNull())
			WifiStation.setIP(AppSettings.ip, AppSettings.netmask, AppSettings.gateway);
	}
	mqtt = new MqttClient(AppSettings.mqtt_server,AppSettings.mqtt_port, onMessageReceived);

	WifiStation.startScan(networkScanCompleted);

	// Start AP for configuration
	WifiAccessPoint.enable(true);
	WifiAccessPoint.config("Sming Configuration "+ WifiStation.getMAC(), "", AUTH_OPEN);

	// Run WEB server
	startWebServer();
	
	//Serial.printf("\r\nCurrently running rom %d.\r\n", slot);
	//Serial.println("Type 'help' and press enter for instructions.");
	//Serial.println();
	//pinMode(12, OUTPUT);
	//pinMode(13, OUTPUT);
	//Serial.setCallback(serialCallBack);
	// set timezone hourly difference to UTC
	SystemClock.setTimeZone(2);

	//printTimer.initializeMs(20000, onPrintSystemTime).start();
	wifi_station_set_hostname(host);
	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start
	//runRx();
}
