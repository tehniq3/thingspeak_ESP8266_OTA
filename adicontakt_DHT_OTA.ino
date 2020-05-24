// www.arduinesp.com
// Plot DTH22 data on thingspeak.com using an ESP8266
// April 11 2015
// Author: Jeroen Beemster
// Website: www.arduinesp.com
/*********
  Rui Santos -  Complete project details at https://randomnerdtutorials.com
  Arduino IDE example: Examples > Arduino OTA > BasicOTA.ino
  https://randomnerdtutorials.com/esp8266-ota-updates-with-arduino-ide-over-the-air/
  
*********/
// combined sketch by Nicu FLORICA (niq_ro) - https://nicuflorica.blogspot.com/

#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// replace with your channel’s thingspeak API key,
String apiKey = "xxxxx";
const char* ssid = "xxxx";
const char* password = "xxxxx";

const char* server = "api.thingspeak.com";
#define DHTPIN 4 // https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/

DHT dht(DHTPIN, DHT22,18);  // // for ESP8266
WiFiClient client;

// analog pin: https://roboindia.com/tutorials/admin/source32145898/tutorials%20Images/nodeMCU_arduino/analog_input/analog_input_nodemcu.jpg
#define analogpin A0
float R1 = 1.;  // resitor beteen GND and A0
float R2 = 3.3;
float ubat = 0; // baterry voltage

float t,h;  // temperature and humidity

void setup() {
Serial.begin(115200);
delay(10);
dht.begin();

WiFi.begin(ssid, password);

Serial.println();
Serial.println();
Serial.print("Connecting to ");
Serial.println(ssid);

WiFi.mode(WIFI_STA);
WiFi.begin(ssid, password);

while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

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
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
void loop() {
 ArduinoOTA.handle();
  
ubat = (float)3.353*analogRead(analogpin)/1024.;
ubat = (R2 + R1) * ubat / R1;
h = dht.readHumidity();
t = dht.readTemperature();
t = t-2.5; // correction
if (isnan(h) || isnan(t)) {
Serial.println("Failed to read from DHT sensor!");
return;
}

if (client.connect(server,80)) { // "184.106.153.149" or api.thingspeak.com
String postStr = apiKey;
postStr +="&field1=";
postStr += String(t);
postStr +="&field2=";
postStr += String(h);
postStr +="&field3=";
postStr += String(ubat);
postStr += "\r\n\r\n";

client.print("POST /update HTTP/1.1\n");
client.print("Host: api.thingspeak.com\n");
client.print("Connection: close\n");
client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
client.print("Content-Type: application/x-www-form-urlencoded\n");
client.print("Content-Length: ");
client.print(postStr.length());
client.print("\n\n");
client.print(postStr);

Serial.print("Temperature: ");
Serial.print(t);
Serial.print(" degrees Celsius Humidity: ");
Serial.print(h);
Serial.print(" % and Voltage battery: ");
Serial.print(ubat);
Serial.println(" V send to Thingspeak");
}
client.stop();

Serial.println("Waiting…");
// thingspeak needs minimum 15 sec delay between updates
delay(20000);
}
