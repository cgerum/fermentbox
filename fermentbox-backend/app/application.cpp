#include <SmingCore.h>

#include "configuration.h"
#include "control_loop.h"
#include "logger.h"
#include "mdns.h"
#include "sensors.h"
#include "webserver.h"

std::unique_ptr<NtpClient> ntpClient;

static void WifiDisconnect(const String &ssid, MacAddress bssid,
                           WifiDisconnectReason reason) {
  debugf("DISCONNECT - SSID: %s, REASON: %s\n", ssid.c_str(),
         WifiEvents.getDisconnectReasonDesc(reason).c_str());

  if (!WifiAccessPoint.isEnabled()) {
    debugf("Starting OWN AP");
    WifiStation.disconnect();
    WifiAccessPoint.enable(true);
    WifiStation.connect();
  }
}

static void WifiGotIP(IpAddress ip, IpAddress mask, IpAddress gateway) {
  debugf("GOTIP - IP: %s, MASK: %s, GW: %s\n", ip.toString().c_str(),
         mask.toString().c_str(), gateway.toString().c_str());

  if (WifiAccessPoint.isEnabled()) {
    debugf("Shutdown OWN AP");
    WifiAccessPoint.enable(false);
  }
  // Add commands to be executed after successfully connecting to AP and got IP
  // from it
  startMDNS();
}

void init() {
  Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
  Serial.systemDebugOutput(true);

  // On Host builds timers are not ready during static init, so create NTP client here.
  if (!ntpClient) {
    ntpClient = std::make_unique<NtpClient>("pool.ntp.org", 3600);
  }

  spiffs_mount(); // Mount file system, in order to work with files

  // SET higher CPU freq & disable wifi sleep
  system_update_cpu_freq(SYS_CPU_160MHZ);
  wifi_set_sleep_type(NONE_SLEEP_T);

  FermentboxConfig activeConfig = FermentboxConfig::load();
  if (activeConfig.Wifi.SSID) {
    WifiStation.enable(true);
    WifiStation.config(activeConfig.Wifi.SSID, activeConfig.Wifi.Password);
  }

  // Attach Wifi events handlers
  WifiEvents.onStationDisconnect(WifiDisconnect);
  WifiEvents.onStationGotIP(WifiGotIP);

  startWebServer();
  startSensors();
  startControlLoop();
  startLogger();
}
