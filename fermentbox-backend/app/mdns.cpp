#include "mdns.h"

#include <SmingCore.h>

#ifdef ARCH_ESP8266
extern "C" {
#include <espconn.h>
}
#endif

void startMDNS() {
#ifdef ARCH_ESP8266
  static mdns_info info;
  static char hostName[] = "fermentbox";
  static char serverName[] = "fermentbox";
  static char txtVersion[] = "version=now";
  IpAddress stationIp = WifiStation.getIP();

  memset(&info, 0, sizeof(info));
  info.host_name = hostName;
  info.ipAddr = stationIp;
  info.server_name = serverName;
  info.server_port = 80;
  info.txt_data[0] = txtVersion;

  espconn_mdns_init(&info);
  espconn_mdns_enable();

  debugf("mDNS started as %s.local on %s", hostName,
         stationIp.toString().c_str());
#else
  debugf("mDNS not started: only supported on ESP8266 target");
#endif
}
