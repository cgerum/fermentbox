#include "configuration.h"
#include <ArduinoJson.h>

// If you want, you can define WiFi settings globally in Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

static FermentboxConfig  Config;

FermentboxConfig& FermentboxConfig::load()
{

	StaticJsonDocument<ConfigJsonBufferSize> doc;
	if(Json::loadFromFile(doc, FERMENTBOX_CONFIG_FILE)) {
		JsonObject network = doc["network"];
		Config.StaSSID = network["StaSSID"].as<const char*>();
		Config.StaPassword = network["StaPassword"].as<const char*>();
		Config.StaEnable = network["StaEnable"];
	} else {
		// Factory defaults if no config file present, or could not access it
		Config.StaSSID = WIFI_SSID;
		Config.StaPassword = WIFI_PWD;
	}
	return Config;
}

FermentboxConfig& FermentboxConfig::get() {
	return Config;
}

void FermentboxConfig::save()
{
	StaticJsonDocument<ConfigJsonBufferSize> doc;

	JsonObject network = doc.createNestedObject("network");
	network["StaSSID"] = Config.StaSSID;
	network["StaPassword"] = Config.StaPassword;
	network["StaEnable"] = Config.StaEnable;

	Json::saveToFile(doc, FERMENTBOX_CONFIG_FILE, Json::Pretty);
}
