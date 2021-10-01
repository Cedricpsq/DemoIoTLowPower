#include <Arduino.h>
#include <Wifi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_wifi.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

const int buttonPin = 2;     // the number of the pushbutton pin

const char* ssid      = "AndroidAP_5446"; // enter name of modem
const char* password  = "MeruCityFuturNewYork60"; // enter pwd of modem
const char* privateKey = "699844a30d3d2c18d0b4108e5b85f157";
const char* host = "api.weatherstack.com";
const char* city = "Paris";
const int httpPort = 80;
bool lock_display = 0;
esp_sleep_wakeup_cause_t wakeup_reason;

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */


LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient client;
RTC_DATA_ATTR int8_t ext_temp = 100;

enum {update_data, sleep_state, display}stateMachine = update_data;

void display_info(){
  lock_display = 1;
  Serial.print("Temperature : ");
  Serial.print(ext_temp);
  Serial.println("°C");
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print(ext_temp);
  lcd.print("C");
  delay(6000);
  lcd.setCursor(0,0);
  lcd.noBacklight();
  lcd.clear();
  lock_display = 0;
}

void setup() {
  // put your setup code here, to run once:
  lcd.init();
  Serial.begin(115200);
  delay(10);
  pinMode(buttonPin, INPUT);
  lcd.noBacklight();
  //----- Adressage matériel -----
  // En cas de non fonctionnement, mettez la ligne 8 en
  // commentaire et retirez le commentaire à la ligne 9.


  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : stateMachine = display; break;
    case ESP_SLEEP_WAKEUP_TIMER : stateMachine = update_data; break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 1);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  attachInterrupt(digitalPinToInterrupt(buttonPin), display_info, HIGH);
  delay(2000);
}

void loop() {


  switch(stateMachine){
    case update_data:
      {
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
            ext_temp = (uint8_t) sensor.toInt();
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
    case display:
      display_info();
      stateMachine = sleep_state;
      break;
    case sleep_state:
      if(lock_display == 0){
        esp_wifi_stop(); // it is necessary to stop wifi modem before light sleep mode
        esp_deep_sleep_start();
      }
      break;
  }
}
