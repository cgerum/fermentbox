#pragma once

#include <SmingCore.h>

const char FERMENTBOX_CONFIG_FILE[] =
    ".therm.conf"; // leading point for security reasons :)

// Application configuration JsonBuffer size, increase it if you have large
// config
const uint8_t ConfigJsonBufferSize = 200;

class FermentboxConfig {
public:
  FermentboxConfig() {}
  struct WifiConfig {
    String SSID;
    String Password;
  };

  WifiConfig Wifi;

  static FermentboxConfig &load();
  static FermentboxConfig &get();
  void save();
};
