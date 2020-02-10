//Increase "define MQTT_MAX_PACKET_SIZE" in PubSubClient.h to something bigger  i.e. 2048

#include "Arduino.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Wire.h>

//Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_DC     D1
#define OLED_CS     D2
#define OLED_RESET  -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Wifi and MQTT

const char* ssid = "IOT";
const char* password = "iot12345678";
const char* mqttServer = "192.168.0.10";
const int mqttPort = 1883;
const char* PowTeleTopic = "tele/sonoff/SENSOR";

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  
  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, payload, length);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
     }
  
  double power = doc["ENERGY"]["Power"];
  Serial.print("Power: ");
  Serial.println(power);

  double voltage = doc["ENERGY"]["Voltage"];
  Serial.print("Voltage: ");
  Serial.println(voltage);



}

void reconnect() {
    while (!client.connected()) {
        Serial.println("Reconnecting MQTT...");
        if (!client.connect("ESP8266Client")) {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying in 5 seconds");
            delay(5000);
        }
    }
    client.subscribe(PowTeleTopic);
    Serial.println("MQTT reonnected...");
}

void setup() {
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.drawChar(5, 10, 'X', WHITE, BLACK, 2);
  display.display();
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to the WiFi network");
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("MQTT connected");  
          } 
    else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
          }
  }
 
  client.publish("esp/test", "Hello from ESP8266");
  client.subscribe("tele/sonoff/SENSOR");
  
}


void loop() {
  if (!client.connected()) {
        reconnect();
    }

  client.loop();
}
