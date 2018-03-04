// Blended Adafruit MQTT Library ESP8266 Example with 
// Adafruit IO Digital Output Example

// Many thanks to Tony DiCola (MQTT Example) for creating the MQTT Example,
// as well as Todd Treece for creating the Adafruit IO Digital Output example 
// on behald of Adafruit Industries
// Copyright (c) 2016 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

/************************** Configuration ***********************************/

#include <stdlib.h>
#include "config.h"
#define RED_LED 5
#define GREEN_LED 4

/************************** Global State ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
// Setup a feed called 'nextbus' for subscribing to changes.
Adafruit_MQTT_Subscribe nextbus = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/nextbus");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  // set led pin as a digital output
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  
  Serial.begin(115200);
  delay(10);

  Serial.println(F("NextBus Notification System"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&nextbus);
}

uint32_t x=0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &nextbus) {
      Serial.print(F("Next bus arrives in: "));
      Serial.print((char *)nextbus.lastread);
      Serial.println(F(" minutes"));
      int minutes = atoi((char *)nextbus.lastread);
     
      if (minutes < 10) {
        digitalWrite(GREEN_LED, LOW); 
        digitalWrite(RED_LED, HIGH);
        Serial.println("Less than 10 minutes");
      }
      if (minutes > 10 && minutes < 25) {
        digitalWrite(GREEN_LED, HIGH); 
        digitalWrite(RED_LED, HIGH);
        Serial.println("Less than 25 minutes");
      }
      if (minutes > 25) {
        digitalWrite(RED_LED, LOW); 
        digitalWrite(GREEN_LED, HIGH);
        Serial.println("More than 25 minutes");
      }
    }
  }

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
