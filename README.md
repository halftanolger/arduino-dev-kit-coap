# Arduino Dev-Kit CoAP Example Code

The example sends CoAP data packets over LTE-M (Cat M1) or NB-IoT network to Telenor Managed IoT Cloud (MIC).

## Change Network Protocol

The code will by default use the LTE-M network.

``` cpp
#define SECRET_RAT       7 // Radio Access Technology (7 is for LTE-M and 8 is for NB-IoT)
```

To use the NB-IoT network, change the `SECRET_RAT` define in [arduino_secrets.h](./arduino_secrets.h):

``` cpp
#define SECRET_RAT       8 // Radio Access Technology (7 is for LTE-M and 8 is for NB-IoT)
```

## Network Related Code Changes

The code in this repository reflects settings for the network in Telenor Norway. If your device will connect to a different network you will have to make some changes in [arduino_secrets.h](./arduino_secrets.h) to reflect this:

``` cpp
// Network related configuration
#define SECRET_PINNUMBER     ""              // SIM card PIN number
#define SECRET_GPRS_APN      "telenor.iotgw" // Telenor IoT Gateway APN
#define SECRET_COAP_IP       0x012010AC      // Telenor IoT Gateway IP address (172.16.32.1)
#define SECRET_COAP_ENDPOINT "/"             // Telenor IoT Gateway CoAP endpoint
#define SECRET_COAP_PORT     5683            // Telenor IoT Gateway CoAP port
#define SECRET_RAT           7               // Radio Access Technology (7 is for LTE-M and 8 is for NB-IoT)
#define SECRET_COPS          24201           // Telenor network shortname

```
