#include "configuration.h"
#include <ArduinoJson.h>

// If you want, you can define WiFi settings globally in Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

static FermentboxConfig Config;

FermentboxConfig &FermentboxConfig::load() {
#ifdef ARCH_HOST
  constexpr bool defaultFakeMode = true;
#else
  constexpr bool defaultFakeMode = false;
#endif

  StaticJsonDocument<ConfigJsonBufferSize> doc;
  if (Json::loadFromFile(doc, FERMENTBOX_CONFIG_FILE)) {
    JsonObject network = doc["Wifi"];
    Config.Wifi.SSID = network["SSID"].as<const char *>();
    Config.Wifi.Password = network["Password"].as<const char *>();
    if (doc.containsKey("FakeMode")) {
      Config.FakeMode = doc["FakeMode"].as<bool>();
    } else {
      Config.FakeMode = defaultFakeMode;
    }
  } else {
    // Factory defaults if no config file present, or could not access it
    Config.Wifi.SSID = WIFI_SSID;
    Config.Wifi.Password = WIFI_PWD;
    Config.FakeMode = defaultFakeMode;
  }
  return Config;
}

FermentboxConfig &FermentboxConfig::get() { return Config; }

void FermentboxConfig::save() {
  StaticJsonDocument<ConfigJsonBufferSize> doc;

  JsonObject network = doc.createNestedObject("Wifi");
  network["SSID"] = Config.Wifi.SSID;
  network["Password"] = Config.Wifi.Password;
  doc["FakeMode"] = Config.FakeMode;

  Json::saveToFile(doc, FERMENTBOX_CONFIG_FILE, Json::Pretty);
}
