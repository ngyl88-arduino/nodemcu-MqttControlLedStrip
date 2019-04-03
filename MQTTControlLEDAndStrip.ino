/*
  [Initial Ref] WiFi Web Server LED Blink

  A simple web server that lets you blink an LED via the web.
  This sketch will print the IP address of your WiFi Shield (once connected)
  to the Serial monitor. From there, you can open that address in a web browser
  to turn on and off the LED on pin 5.

  If the IP address of your shield is yourAddress:
  http://yourAddress/H turns the LED on
  http://yourAddress/L turns it off

  This example is written for a network using WPA encryption. For
  WEP or WPA, change the Wifi.begin() call accordingly.

  Circuit:
   WiFi shield attached
   LED attached to pin 5

  created for arduino 25 Nov 2012
  by Tom Igoe

  ported for sparkfun esp32
  31.01.2017 by Jan Hendrik Berlin

  [MQTT Ref]
  https://github.com/256dpi/arduino-mqtt
  https://www.teachmemicro.com/nodemcu-mqtt-tutorial
  https://shiftr.io/try#terminal (curl -X POST "http://try:try@broker.shiftr.io/nodemcu/test" -d "Hi from Terminal")
*/

#include "secret.h"

#include <FastLED.h>
#include <WiFi.h>
#include <MQTTClient.h> // https://github.com/256dpi/arduino-mqtt

#define LED_BUILTIN 2

#define NUM_LEDS 1
#define DATA_PIN 18
CRGB leds[NUM_LEDS];

WiFiServer server(80);
WiFiClient wifiClient;
MQTTClient mqttClient;

void connectMqtt()
{
  Serial.println();
  Serial.print("Connecting to MQTT broker...\nClient Id:");
  Serial.println(mqttClientId);

// skip: skip network level connection and jump to the MQTT level connection
//  bool connect(const char clientId[], bool skip = false);
//  bool connect(const char clientId[], const char username[], bool skip = false);
//  bool connect(const char clientId[], const char username[], const char password[], bool skip = false);
//  while (!mqttClient.connect(mqttClientId, mqttUsername, mqttPassword)) {         // actual MQTT call
  while (!mqttClient.connect("arduino", "try", "try")) {                          // On https://shiftr.io/try, client.connect(name, key, secret)
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("MQTT connected.");

  mqttClient.subscribe("nodemcu");
  // mqttClient.unsubscribe("nodemcu");
}

void messageReceived(String &topic, String &payload)
{
  Serial.println("incoming MQTT message: " + topic + " - " + payload);
}

void connectWiFi()
{
  Serial.println();
  Serial.print("Connecting to wifi...\nSSID: ");
  Serial.println(ssid);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.\nIP address: "+ WiFi.localIP());
}

void setup()
{
  pinMode (LED_BUILTIN, OUTPUT);
  pinMode (DATA_PIN, OUTPUT);
  
  FastLED.addLeds<WS2811, DATA_PIN, GBR>(leds, NUM_LEDS);

  Serial.begin(115200);
  delay(10);

  // Initialize WiFi

  WiFi.begin(ssid, password);
  connectWiFi();
  
  // Initialize MQTT client
  
  mqttClient.begin("broker.shiftr.io", wifiClient);
  mqttClient.onMessage(messageReceived);

  connectMqtt();

  // Start serving FASTLED request
  server.begin();

}

void loop() {

  // MQTT : begin
  
  mqttClient.loop();
//  delay(10);                                // for stability on ESP8266

  if (!mqttClient.connected()) {
    connectMqtt();
  }

  // FASTLED : begin
  
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("LED is defined on pin <b>" + String(DATA_PIN) + "</b><br>");
            client.print("Click <a href=\"/red\">here</a> to turn the LED on<br>");
            client.print("Click <a href=\"/green\">here</a> to turn the LED off<br>");
            client.print("Click <a href=\"/blue\">here</a> to turn the LED off<br>");
            client.print("Click <a href=\"/off\">here</a> to turn the LED on pin <b>" + String(DATA_PIN) + "</b> off.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /red")) {
          Serial.println("--> High");
          digitalWrite(LED_BUILTIN, HIGH);               // GET /H turns the LED on
          delay(50);
          leds[0] = CRGB::Red;
          FastLED.show();
          delay(500);
        }
        if (currentLine.endsWith("GET /green")) {
          Serial.println("--> Low");
          digitalWrite(LED_BUILTIN, LOW);                // GET /L turns the LED off
          delay(50);
          leds[0] = CRGB::Green;
          FastLED.show();
          delay(500);
        }
        if (currentLine.endsWith("GET /blue")) {
          Serial.println("--> Low");
          digitalWrite(LED_BUILTIN, LOW);                // GET /L turns the LED off
          delay(50);
          leds[0] = CRGB::Blue;
          FastLED.show();
          delay(500);
        }
        if (currentLine.endsWith("GET /off")) {
          Serial.println("--> Off");
          leds[0] = CRGB::Black;
          FastLED.show();
          delay(500);
        }
      }
    }
    
    client.stop();
    Serial.println("Client Disconnected.");
  }
}
