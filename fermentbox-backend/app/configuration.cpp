#include "configuration.h"
#include <ArduinoJson.h>

// If you want, you can define WiFi settings globally in Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

static FermentboxConfig Config;

FermentboxConfig &FermentboxConfig::load() {

  StaticJsonDocument<ConfigJsonBufferSize> doc;
  if (Json::loadFromFile(doc, FERMENTBOX_CONFIG_FILE)) {
    JsonObject network = doc["Wifi"];
    Config.Wifi.SSID = network["SSID"].as<const char *>();
    Config.Wifi.Password = network["Password"].as<const char *>();
  } else {
    // Factory defaults if no config file present, or could not access it
    Config.Wifi.SSID = WIFI_SSID;
    Config.Wifi.Password = WIFI_PWD;
  }
  return Config;
}

FermentboxConfig &FermentboxConfig::get() { return Config; }

void FermentboxConfig::save() {
  StaticJsonDocument<ConfigJsonBufferSize> doc;

  JsonObject network = doc.createNestedObject("Wifi");
  network["SSID"] = Config.Wifi.SSID;
  network["Password"] = Config.Wifi.Password;

  Json::saveToFile(doc, FERMENTBOX_CONFIG_FILE, Json::Pretty);
}
