
/*
    test on your server with
    $ mosquitto_sub -h 127.0.0.1 -i testSub -t outLicks

*/

/////////////////////////////////////////////////////////////////////////////
//wifi manager     https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

/////////////////////////////////////////////////////////////////////////////
//irRemote code https://github.com/markszabo/IRremoteESP8266/wiki#ir-receiving
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
// An IR detector/demodulator is connected to GPIO pin 14(D5 on a NodeMCU).
uint16_t RECV_PIN = 14;
IRrecv irrecv(RECV_PIN);
decode_results results;

/////////////////////////////////////////////////////////////////////////////
//mqtt broker https://github.com/knolleary/pubsubclient
#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

const char* mqtt_server = "idiot.io";

///////////////////////////////////////////////
// pysicals
#define ledStatus 2

void setup() {
  Serial.begin(115200);
  Serial.println("starting..");

  //wifi manger
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("connected...yeey :)");

  //IRrecv
  irrecv.enableIRIn();  // Start the receiver

  //MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(ledStatus, OUTPUT);
  digitalWrite(ledStatus, LOW);

}

void loop() {
  //mqtt loop
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //ir recv callback
  if (irrecv.decode(&results)) {
    digitalWrite(ledStatus, HIGH);

    //mqtt

    String payload = "{\"licked\":";
    /*
      payload += uint64ToString(uint64_t results.value(),uint8_t base) ;
      payload += ",\"millis\":";
      payload += millis();
      payload += "}";
    */

    if (client.publish("outLicks", (char*) payload.c_str())) {
      Serial.println("Publish ok");
    }
    else {
      Serial.println("Publish failed");
    }

    /*
      ++value;
      snprintf (msg, 128, "licked %lld @ %ld", results.value, millis() );


        Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("outLicks", msg);

      // print() & println() can't handle printing long longs. (uint64_t)
      serialPrintUint64(results.value, HEX);
      Serial.println("");

    */
    delay(100);

    irrecv.resume();  // Receive the next value
  }


  digitalWrite(ledStatus, LOW);

}

//MQTT functions
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
