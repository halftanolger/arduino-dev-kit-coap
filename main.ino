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

const char   PINNUMBER[]  = "1111";     // SIM card PIN number
const char   APN[]        = "mda.ee";   // Telenor IoT Gateway APN (telenor.iotgw)
unsigned int MIC_UDP_PORT = 1234;       // Local port to listen for UDP packets
unsigned int COPS         = 24201;      // Telenor network shortname
IPAddress    MIC_IP(172, 16, 15, 14);   // Telenor IoT Gateway IP address
byte         packetBuffer[512];

NB nbAccess;
GPRS gprs;
NBUDP Udp;

void setup() {
  // Reserve space to hold response from modem
  String response = "";
  response.reserve(100);

  // Open serial communication and wait for port to open
  Serial.begin(115200);
  while (!Serial);

  // Wait for modem to get ready
  int modemStatus;
  for (modemStatus = MODEM.begin(true); modemStatus != 1; modemStatus = MODEM.begin(true)) {
    Serial.println("Modem not ready, retrying in 2s...");
    delay(2000);
  }

  // Set radio technology
  Serial.print("Set radio technology to NB-IoT or Cat-M1 (7 is for Cat M1 and 8 is for NB-IoT): ");
  MODEM.send("AT+URAT=7");
  MODEM.waitForResponse(100, &response);
  Serial.println(response);
  Serial.println("done.");

  // Turn modem on
  Serial.println("Modem ready, turn radio on in order to configure it...");
  MODEM.send("AT+CFUN=1");
  MODEM.waitForResponse(100, &response);
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
  MODEM.sendf("AT+COPS=1,2,\"%s\"", COPS);
  MODEM.waitForResponse(2000, &response);
  Serial.println(response);
  Serial.println("done.");

  // Set APN and check if network is ready
  Serial.println("Setup APN...");
  NB_NetworkStatus_t networkStatus;
  for (networkStatus = nbAccess.begin(PINNUMBER, APN); networkStatus != NB_READY; networkStatus = nbAccess.begin(PINNUMBER, APN)) {
    Serial.println("Network not ready, retrying in 2s...");
    delay(2000);
  }
  Serial.println("done.");

  Serial.println("Try to connect to IP network (set up PDP context)...");
  boolean connected = false;
  while (!connected) {
    if (gprs.attachGPRS() == GPRS_READY) {
      Serial.println("Connected!");
      connected = true;
    } else {
      Serial.println("Not connected, retrying in 1s...");
      delay(1000);
    }
  }
  Serial.println("done.");

  Serial.println("Setup socket for connection to MIC...");
  Udp.begin(MIC_UDP_PORT);

  // Seed random number generator with noise from pin 0
  randomSeed(analogRead(0));
}

void loop() {
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
  Udp.beginPacket(MIC_IP, MIC_UDP_PORT);
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
