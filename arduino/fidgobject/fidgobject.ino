// Basic demo for accelerometer readings from Adafruit MSA301

#include <Wire.h>
#include <Adafruit_MSA301.h>
#include <Adafruit_Sensor.h>
#include "config_test.h"
#include "certificates.h"
#include <PromLokiTransport.h>
#include <GrafanaLoki.h>
#include <Math.h>

// Change these to change your tracked projects
#define P1 "Loki Ops"
#define P2 "Loki Community"
#define P3 "Loki"
#define P4 "Hiring"
#define P5 "Sales"
#define P6 "1-1"
#define P7 "Management"
#define P8 "BAU"


// These set the thresholds used to know which way gravity is pointing and thus which side is up, 
// shouldn't need to change unless your object has a different number of sides
#define ON_MIN 8
#define	ON_MAX 11
#define	OFF_MIN -1
#define	OFF_MAX 1
#define	HALF_MIN 5
#define	HALF_MAX 8
#define	Z_THRESH 5

// Create the accelerometer objects
Adafruit_MSA301 msa;
TwoWire MSATW = TwoWire(0);

// Create a transport and client object for sending our data.
PromLokiTransport transport;
LokiClient client(transport);


// Create our stream for entries
LokiStream tf(2, 100, "{job=\"timefidget\",type=\"add\"}");
LokiStreams streams(1);

const char* id = "w1";
const char* formatString = "id=\"%s\" type=add pos=%s project=\"%s\"";

void setup(void) {
  MSATW.begin(15, 13, 100000);
  Serial.begin(115200);
  while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Starting fidgobject");

  // Try to initialize!
  if (!msa.begin(MSA301_I2CADDR_DEFAULT, &MSATW)) {
    Serial.println("Failed to find MSA301 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MSA301 Found and connected");
  msa.setDataRate(MSA301_DATARATE_1_HZ);

  transport.setWifiSsid(WIFI_SSID);
  transport.setWifiPass(WIFI_PASSWORD);
  transport.setUseTls(true);
  transport.setCerts(grafanaCert, strlen(grafanaCert));
  transport.setDebug(Serial);  // Remove this line to disable debug logging of the transport layer. 
  if (!transport.begin()) {
    Serial.println(transport.errmsg);
    while (true) {};
  }

  // Configure the client
  client.setUrl(GC_URL);
  client.setPath(GC_PATH);
  client.setPort(GC_PORT);
  client.setUser(GC_USER);
  client.setPass(GC_PASS);

  client.setDebug(Serial); // Remove this line to disable debug logging of the client.
  if (!client.begin()) {
    Serial.println(client.errmsg);
    while (true) {};
  }

  // Add our stream objects to the streams object
  streams.addStream(tf);
  streams.setDebug(Serial);  // Remove this line to disable debug logging of the write request serialization and compression.


}

void sendToLoki(const char* pos, const char* projectName) {
  char str1[100];
  snprintf(str1, 100, formatString, id, pos, projectName);
  if (!tf.addEntry(client.getTimeNanos(), str1, strlen(str1))) {
    Serial.println(tf.errmsg);
  }
  Serial.print("Sending Project: ");
  Serial.println(projectName);
  LokiClient::SendResult res = client.send(streams);
  if (res != LokiClient::SendResult::SUCCESS) {
    Serial.println("Failed to send to Loki");
    if (client.errmsg) {
      Serial.println(client.errmsg);
    }
    if (transport.errmsg) {
      Serial.println(transport.errmsg);
    }
  }
  // Reset Streams
  tf.resetEntries();
}


void loop() {

  // Get new accel event
  sensors_event_t event;
  msa.getEvent(&event);

  float x = event.acceleration.x;
  float y = event.acceleration.y;
  float z = event.acceleration.z;

  if (abs(z) > Z_THRESH) {
    // Off
    //level.Info(util.Logger).Log("pos", "0")
  }
  else if (x > OFF_MIN && x < OFF_MAX && y < -ON_MIN && y > -ON_MAX) {
    // Position 1
    sendToLoki("1", P1);
  }
  else if (x > HALF_MIN && x < HALF_MAX && y < -HALF_MIN && y > -HALF_MAX) {
    // Position 2
    sendToLoki("2", P2);
  }
  else if (x > ON_MIN && x < ON_MAX && y > OFF_MIN && y < OFF_MAX) {
    // Position 3
    sendToLoki("3", P3);
  }
  else if (x > HALF_MIN && x < HALF_MAX && y > HALF_MIN && y < HALF_MAX) {
    // Position 4
    sendToLoki("4", P4);
  }
  else if (x > OFF_MIN && x < OFF_MAX && y > ON_MIN && y < ON_MAX) {
    // Position 5
    sendToLoki("5", P5);
  }
  else if (x < -HALF_MIN && x > -HALF_MAX && y > HALF_MIN && y < HALF_MAX) {
    // Position 6
    sendToLoki("6", P6);
  }
  else if (x < -ON_MIN && x > -ON_MAX && y > OFF_MIN && y < ON_MIN) {
    // Position 7
    sendToLoki("7", P7);
  }
  else if (x < -HALF_MIN && x > -HALF_MAX && y < -HALF_MIN && y > -HALF_MAX) {
    // Position 8
    sendToLoki("8", P8);
  }

  delay(5000);
}
