#include "mdns.h"

#include <SmingCore.h>

void startMDNS() {
  struct mdns_info *info =
      (struct mdns_info *)os_zalloc(sizeof(struct mdns_info));
  info->host_name =
      (char *)"fermentbox"; // You can replace test with your own host name
  info->ipAddr = WifiStation.getIP();
  info->server_name = (char *)"Sming";
  info->server_port = 80;
  info->txt_data[0] = (char *)"version = now";
  espconn_mdns_init(info);
}
