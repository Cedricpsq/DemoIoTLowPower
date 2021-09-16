#include <Arduino.h>
#include <Wifi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_wifi.h"

const int buttonPin = 2;     // the number of the pushbutton pin

const char* ssid      = "-------------------"; // enter name of modem
const char* password  = "-------------------"; // enter pwd of modem
const char* privateKey = "699844a30d3d2c18d0b4108e5b85f157";
const char* host = "api.weatherstack.com";
const char* city = "Paris";
const int httpPort = 80;
WiFiClient client;

enum {sleep_state, awake_state}stateMachine = awake_state;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);
  pinMode(buttonPin, INPUT);
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid,password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 1);
  //attachInterrupt(digitalPinToInterrupt(buttonPin), wake_up, HIGH);
  delay(2000);
}

void loop() {


  switch(stateMachine){
    case awake_state:
      {
      // put your main code here, to run repeatedly:
      Serial.print("connecting to ");
      Serial.println(host);

      // Use WiFiClient class to create TCP connections


      if (!client.connect(host, httpPort)) {
          Serial.println("connection failed");
          delay(2000);
          return;
      }

      // We now create a URI for the request
      String url = "/current";
      url += "?access_key=";
      url += privateKey;
      url += "&query=";
      url += city;

      Serial.print("Requesting URL: ");
      Serial.println(url);

      // This will send the request to the server
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Connection: close\r\n\r\n");
      unsigned long timeout = millis();
      while (client.available() == 0) {
          if (millis() - timeout > 5000) {
              Serial.println(">>> Client Timeout !");
              client.stop();
              return;
          }
      }

      // Read all the lines of the reply from server and print them to Serial
      while(client.available()) {
          String line = client.readStringUntil('\r');
          DynamicJsonDocument doc(1024);
          deserializeJson(doc, line);
          String sensor = doc["current"]["temperature"];
          if(sensor != "null") {
            Serial.print("Temperature in Paris : ");
            Serial.println(sensor + "°C");
          }
      }



      Serial.println();
      Serial.println("closing connection");
      Serial.println("Bed time !");
      Serial.flush();
      client.stop();
      stateMachine = sleep_state;
      break;
      }
    case sleep_state:
      esp_wifi_stop(); // it is necessary to stop wifi modem before light sleep mode
      esp_deep_sleep_start();
      esp_wifi_start(); // we need to restart and reconnect after sleep
      WiFi.begin(ssid,password);
      while (WiFi.status() != WL_CONNECTED) {
          delay(500);
      }
      stateMachine = awake_state;
      break;
  }

}
