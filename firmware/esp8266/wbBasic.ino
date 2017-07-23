/*
Copyright (C) 2017 by Giovanni Di Mingo <pino_otto@yahoo.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


/*
 * Use a pushbutton to toggle the onboard LED and send a HTTP request.
 *
 * If you do not have the 1 Button Shield, add a pushbutton or switch between D3 and GND.
 * 
 * This sketch sends data via HTTP GET requests.
 * 
 */

#include <ESP8266WiFi.h>          // ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>            // Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     // Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* ssid      = "your_SSID";
const char* password  = "your_wifi_password";
const char* host      = "192.168.0.10";
const int   httpPort  = 8080;   // test
//const int   httpPort  = 8081;   // production

// the static IP address for the esp:
IPAddress ip(192, 168, 0, 180);                 // wifi push button (wb)
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress DNS(192, 168, 0, 1);

int inputPin  = D3;            // pushbutton connected to digital pin D3
int ledPin    = BUILTIN_LED;   // onboard LED
int inputVal  = 0;             // variable to store the read value
int state     = 0;             // state of the state machine


/*******************************
 * request
 */
String request(String url) {

  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return "-1";
  }
  
  // We now create a URI for the request
//  String url = "/filehelper";
//  url += streamId;
//  url += "?private_key=";
//  url += privateKey;
//  url += "&value=";
//  url += value;
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) { // waiting for response
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return "-2";
    }
  }

  String response = "";
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    response += client.readStringUntil('\r');
  }
  Serial.print(response);
  
  Serial.println();
  Serial.println("closing connection");

  return response;
  
}


/*******************************
 * setup
 */
void setup() {

  // set the direction of the pins
  
  pinMode(ledPin, OUTPUT);        // set LED as output
  pinMode(inputPin, INPUT);       // set pin as input
  
  // start serial connection
  
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println("Booting");

  // initialize WiFi Manager
  WiFiManager wifiManager;

  // set static IP address
  wifiManager.setSTAStaticIPConfig(ip, gateway, subnet);
  
  // first parameter is name of access point, second is the password
  //wifiManager.autoConnect("PushButton", "esp123456");
  //use an auto generated name from 'ESP' and the esp's Chip ID
  //wifiManager.autoConnect();

  if (!wifiManager.autoConnect("PushButton", "esp123456")) {
    Serial.println("failed to connect, we should reset and see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  // connect to WiFi network

//  Serial.println();
//  Serial.println();
//  Serial.print("Connecting to ");
//  Serial.println(ssid);
//
//  WiFi.config(ip, gateway, subnet, DNS);
//  WiFi.begin(ssid, password);
//  
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    Serial.print(".");
//  }

  // setup OTA

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // flash the LED
  
  digitalWrite(ledPin, LOW);
  delay(500);
  digitalWrite(ledPin, HIGH);
  delay(500);

  delay(5000);

  // check for reset
  inputVal = digitalRead(inputPin);     // read the input pin (for resetting wifi)
  // check if button pushed
  if (inputVal == 0) { // pushed
    WiFi.disconnect(true);
    Serial.println("reset");
    digitalWrite(ledPin, LOW);
    delay(5000);
    ESP.restart();
  }
  
  Serial.println("state 0");
  
}


/*******************************
 * loop
 */
void loop() {

  // handle OTA
  ArduinoOTA.handle();
  
  /**
   * state machine:
   *   - 0: (switch = 1) led off, if switch = 0 --> state = 1
   *   - 1: (switch = 0) led flashing for 5 sec, if switch = 0 --> state = 2, if switch = 1 --> state = 0
   *   - 2: (switch = 0) send msg to wechat, led on --> state = 3
   *   - 3: (switch = 0) led on, if switch = 1 --> state = 0
   */
  inputVal = digitalRead(inputPin);     // read the input pin
  //Serial.println(inputVal);

  String response = "";

  switch (state) {
    
    case 0:
      // turn off the LED
      digitalWrite(ledPin, HIGH);
      // check if button pushed
      if (inputVal == 0) { // pushed
        state = 1;
        Serial.println("state 0 --> 1");
      }
      break;
      
    case 1:
      // flash the LED
      for (int i=0; i < 5; i++) { // for 5 sec
        digitalWrite(ledPin, LOW);
        delay(500);
        digitalWrite(ledPin, HIGH);
        delay(500);
        inputVal = digitalRead(inputPin);   // read again the input pin
        // check if button released
        if (inputVal == 1) { // released
          break;
        }
      }
      inputVal = digitalRead(inputPin);     // read again the input pin (for debouncing)
      // check if button pushed
      if (inputVal == 0) { // pushed
        state = 2;
        Serial.println("state 1 --> 2");
      } else { // released
        state = 0;
        Serial.println("state 1 --> 0");
      }
      break;
      
    case 2:
      Serial.println("sending msg to wechat");
      // send msg to wechat
      response = request("/water");
      Serial.println("response: " + response);
      if (response == "-1" || response == "-2") {
        state = 1;
        Serial.println("state 2 --> 1");
        break;
      }
      // turn on the LED
      digitalWrite(ledPin, LOW);
      state = 3;
      Serial.println("state 2 --> 3");
      break;
      
    case 3:
      // wait for button released
      // turn on the LED
      digitalWrite(ledPin, LOW);
      if (inputVal == 1) { // released
        state = 0;
        Serial.println("state 3 --> 0");
      }
      break;
      
    default:
      // error: unknow state
      break;
      
  }
  
  //digitalWrite(ledPin, inputVal);  // sets the LED to the button's value
  
}

