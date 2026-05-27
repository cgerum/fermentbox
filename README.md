

## Backend Host API Tests

Run the backend endpoint integration tests against the Sming Host binary:

```bash
make test-backend-api-host
```

The test command will:

1. Build `fermentbox-backend` for `SMING_ARCH=Host`
2. Flash the host virtual flash image
3. Start `out/Host/debug/firmware/app` with Sming TAP network emulation
4. Execute HTTP checks for `/getConfig`, `/getMeasurement`, `/getStatus`, `/networkConfig`, and `/schedule/*`

Requirements:

- A TAP interface (`tap0` by default) with `192.168.13.1/24`
- `/dev/net/tun` available in the devcontainer

These are configured by `.devcontainer/devcontainer.json` (`NET_ADMIN` + `/dev/net/tun`) and `.devcontainer/post-start.sh`.


## Notes

### Pin Mappings

According to https://techtutorialsx.com/2017/04/02/esp8266-nodemcu-pin-mappings/ the NodeMCU pin mappings are:

    D0 = GPIO16; DHT
    D1 = GPIO5;
    D2 = GPIO4;
    D3 = GPIO0;
    D4 = GPIO2;
    D5 = GPIO14;
    D6 = GPIO12;
    D7 = GPIO13;
    D8 = GPIO15;
    D9 = GPIO3;
    D10 = GPIO1;
    LED_BUILTIN = GPIO16 