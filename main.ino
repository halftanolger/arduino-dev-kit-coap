/*
 * Arduino Dev-Kit UDP Example Code.
 * 
 * The example sends UDP data packets over LTE-M (Cat M1)
 * or NB-IoT network to Telenor Managed IoT Cloud (MIC).
 * 
 * For help and other code examples:
 * Telenor Start IoT, https://startiot.telenor.com/
 */

#include <MKRNB.h>

#include "arduino_secrets.h"

// Enter sensitive data and other configurations in "arduino_secrets.h".
const char   pinnumber[]      = SECRET_PINNUMBER;
const char   gprs_apn[]       = SECRET_GPRS_APN;
unsigned int udp_port         = SECRET_UDP_PORT;
unsigned int rat              = SECRET_RAT;
unsigned int cops             = SECRET_COPS;

// Telenor IoT Gateway IP address
IPAddress iotgw_ip(172, 16, 15, 14);
byte packetBuffer[512];

NB nbAccess;
GPRS gprs;
NBUDP Udp;

void setup() {
  String response;

  // Reset the ublox module
  pinMode(SARA_RESETN, OUTPUT);
  digitalWrite(SARA_RESETN, HIGH);
  delay(100);
  digitalWrite(SARA_RESETN, LOW);

  // Open serial communication and wait for port to open
  Serial.begin(115200);
  while (!Serial);

  // Wait for modem to get ready
  Serial.println("Waiting for modem to get ready...");
  MODEM.begin();
  while (!MODEM.noop());
  Serial.println("done.");

  // Disconnect from any networks
  Serial.print("Disconnecting from network...");
  MODEM.sendf("AT+COPS=2");
  MODEM.waitForResponse(2000, &response);
  Serial.println(response);
  Serial.println("done.");

  // Set Radio Access Technology (RAT)
  Serial.print("Set radio technology to NB-IoT or Cat-M1 (7 is for LTE-M and 8 is for NB-IoT)...");
  MODEM.sendf("AT+URAT=%s", rat);
  MODEM.waitForResponse(100, &response);
  Serial.println(response);
  Serial.println("done.");

  // Apply changes
  Serial.print("Applying changes and saving configuration...");
  MODEM.sendf("AT+CFUN=15");
  MODEM.waitForResponse(5000);
  delay(5000);

  while (MODEM.waitForResponse(1000) != 1) {
    delay(1000);
    MODEM.noop();
  }
  Serial.println("done.");

  // Turn modem on
  Serial.println("Modem ready, turn radio on in order to configure it...");
  MODEM.send("AT+CFUN=1");
  MODEM.waitForResponse(2000, &response);
  Serial.println(response);
  Serial.println("done.");

  // Wait for a good signal strength (between 0 and 98)
  Serial.println("Check attachment until CSQ RSSI indicator is less than 99...");
  int status = 99;
  while (status > 98 && status > 0) {
    MODEM.send("AT+CSQ");
    MODEM.waitForResponse(2000, &response);

    String sub = response.substring(6, 8);
    status = sub.toInt(); // Will return 0 if no valid number is found
    delay(1000);
  }
  Serial.println("done.");

  // Set operator to Telenor
  Serial.println("Set operator to Telenor...");
  MODEM.sendf("AT+COPS=1,2,\"%s\"", cops);
  MODEM.waitForResponse(2000, &response);
  Serial.println(response);
  Serial.println("done.");

  // Set APN and check if network is ready
  connectNB();

  Serial.println("Setup socket for connection to MIC...");
  Udp.begin(udp_port);

  // Seed random number generator with noise from pin 0
  randomSeed(analogRead(0));
}

void loop() {
  // Check if connected and if not, reconnect
  if (nbAccess.status() != NB_READY || gprs.status() != GPRS_READY) {
    connectNB();
  }

  Serial.print("Send packet to MIC...");
  sendPacket();

  Serial.println("Check if we have received something...");
  if (receivePacket() > 0) {
    Serial.println("Received packet...");
    String bufferString = String((char *) packetBuffer);
    Serial.println("Packet data is: <" + bufferString + ">");
  } else {
    Serial.println("No data received...");
  }

  // Wait 30 seconds before sending again
  Serial.println("Wait 30s before sending again...");
  delay(30000);
}

void connectNB() {
  Serial.println("Attempting to connect to the cellular network");

  while ((nbAccess.begin(pinnumber) != NB_READY) ||
         (gprs.attachGPRS(gprs_apn) != GPRS_READY)) {
    // failed, retry
    Serial.print(".");
    delay(1000);
  }

  Serial.println("You're connected to the cellular network!");
}

// Send a JSON formatted packet to MIC
unsigned long sendPacket () {
  String p1, p2, p4, payload = "";
  float hum, tmp, r = 0.0;

  hum = 60;
  r = random(0, 9);
  r = r / 10;
  hum = hum + r;
  p1 = hum;
  tmp = 20;
  r = random(0, 9);
  r = r / 10;
  tmp = tmp + r;
  p2 = tmp;

  payload = "{\"temperature\":\"" + p2 + "\", \"humidity\":\"" + p1 + "\",\"latlng\": \"69.681812, 18.988209\"}";
  Serial.println("payload is: " + payload);
  Udp.beginPacket(iotgw_ip, udp_port);
  Udp.write(payload.c_str(), payload.length());
  Udp.endPacket();
}

// Read a packet from the UDP port
int receivePacket () {
  int size = Udp.parsePacket();

  // Check if size is larger than 0, if yes we have received something
  if (size > 0) {
    Serial.println("Packet received!");
    Udp.read(packetBuffer, size); // Read the packet into the buffer

    return size;
  }

  return 0;
}
