/* 
 * Copyright (c) 2019. Yosef Adi Sulistyo 
 * All right reserved.
 * 
 * This program is designed for ESP8266 system
 * Tested on Wemos D1 R2
 * 
 * Science Expo 2019
 */

/* Include arduino libraries */
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h> 
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EEPROM.h>

/* initialize public variable */
#define ONE_WIRE_PIN D2 //init onewire on pin GPIO4 (D2)

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);

ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(80);
//WiFiServer server(80);

const long utcOffsetInSeconds = 25200;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.google.com", utcOffsetInSeconds);

//String header;

String tsensdata; 
String days;
String hours;
String minutes;
String seconds;
String formattime;
String maxTemp;
String maxTempVal;

/* Initialize EEPROM Variable */
int addr = 0;

void handleRoot();
void handleNotFound();
void handleSavedSetup();

void init_wifi() {
  Serial.print("Init WiFi begin....");;
  WiFi.softAPdisconnect(false);
  WiFi.enableAP(false);
  wifiMulti.addAP("scienceexpotest", "padmanaba75");
  wifiMulti.addAP("Science Expo 2019", "padmanaba");
  int i = 0;
  //WiFi.begin(ssid, password);

  // Wait for connection
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.print("   DONE!");
  Serial.println(" ");
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void init_webserver() {
  Serial.println(" ");
  Serial.print("Init Webserver....");
  server.begin();
  
  server.on("/", handleRoot); 
  server.on("/setup", handleSetup);
  server.on("/saved", handleSavedSetup);
  server.onNotFound([](){
    server.send(404, "text/plain", "404: Not found");
  });

  Serial.print("     DONE!");
  Serial.println(" ");
  Serial.println(" ");
}

String prepareHTML() {
  String htmlPage =
    String("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n") +
            "<link rel=\"icon\" href=\"data:,\">\r\n" +
            "<meta http-equiv=\"refresh\" content=\"5\">\r\n" +
            "<head>\r\n" +
            "<body>\r\n" +
            "<h1>Monitoring Industri Perikanan</h1>\r\n" +
            "<h3>Current Time:</h3>\r\n" +
            //"<p>" + days + ", " + hours + ":" + minutes + ":" + seconds + "</p>\r\n" +
            "<p>" + days + ", " + formattime + "</p>\r\n" +
            "<h3>Temperature:</h3>\r\n" +
            "<p>" + tsensdata + "C </p>\r\n" +
            "<br><br>\r\n<p>Developed with love by yosefadi and Arka Lilang W.</p>\r\n" +
            "<p>Halaman ini akan otomatis di-refresh setiap 5 detik</p>\r\n" +
            "</body>\r\n" +
            "\r\n";
  return htmlPage;
}

String prepareSetup() {
  String setupPage =
    String("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n") +
            "<link rel=\"icon\" href=\"data:,\">\r\n" +
            "<head>\r\n" +
            "<body>\r\n" +
            "<h1>Pengaturan Alat</h1>\r\n" +
            "<p>Temperatur Air Maksimum</p>\r\n" +
            "<form METHOD=\"post\"action=\"/saved\">\r\n" +
            "<input type=\"text\" name=\"myText\" value=\"" + maxTempVal + "\">\r\n" +
            "<input type=\"submit\" value=\"Save\">\r\n" +
            "</form>\r\n" +
            "</body>\r\n" +
            "\r\n";
     return setupPage;
}

void setup() {
  // init system
  Serial.begin(115200);
  Serial.println("Hello! NOT FOR SALE!!!");
  Serial.println("yosefadi & arka0309");
  Serial.println(" ");
  pinMode(12, OUTPUT); // cooler pin
  pinMode(13, OUTPUT); // heater pin
  EEPROM.begin(512);

  // NC relay
  digitalWrite(13, HIGH); 
  digitalWrite(12, HIGH);
  
  // Init WiFi
  init_wifi();

  // init WebServer
  init_webserver();
  timeClient.begin();
}
 
void loop() {
  sensors.requestTemperatures();
  // don't take any action while sensor isn't responding
  /*while (sensors.getTempCByIndex(0) == -127.00) {
    digitalWrite(13, HIGH);
    delay(100);
    sensors.requestTemperatures();
    Serial.println("Sensors give invalid value. Requesting temperature again!");
  }*/
  tsensdata = sensors.getTempCByIndex(0); 
  Serial.println("Temperature:");
  Serial.print(tsensdata);
  Serial.println(" ");

  maxTempVal = EEPROM.read(addr+1);
  Serial.println("Max Temp: ");
  Serial.print(maxTempVal);

  float heatoff = maxTempVal.toInt() - 4;
  float heaton = maxTempVal.toInt() - 5;
  float cooloff = maxTempVal.toInt() - 3;
  float coolon = maxTempVal.toInt() + 4;
  
  // heater action
  if (tsensdata.toInt() > heatoff) {
    digitalWrite(13, HIGH); 
  } else if ( tsensdata.toInt() < heaton ) {
    digitalWrite(13, LOW); 
  } 

  // cooler action
  if (tsensdata.toInt() > coolon) {
    digitalWrite(12, LOW);
  } else if ( tsensdata.toInt() < cooloff ) {
    digitalWrite(12, HIGH);
  }
  
  if (wifiMulti.run() != WL_CONNECTED) {
    init_wifi();
  } else {
    days = daysOfTheWeek[timeClient.getDay()];
    hours = timeClient.getHours();
    minutes = timeClient.getMinutes();
    seconds = timeClient.getSeconds();
    formattime = timeClient.getFormattedTime();

    Serial.println(" ");
    Serial.print(days);
    Serial.print(", ");
    Serial.print(hours);
    Serial.print(":");
    Serial.print(minutes);
    Serial.print(":");
    Serial.println(seconds);
    delay(100);
    server.handleClient();
  }
}

void handleRoot() {
  server.send(200, "text/html", prepareHTML());
}

void handleSetup() {
  server.send(200, "text/html", prepareSetup());
}

String prepareSaved() {
  String savedPage =
    String("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n") +
            "<link rel=\"icon\" href=\"data:,\">\r\n" +
            "<head>\r\n" +
            "<body>\r\n" +
            "<h1>Settings Saved!</h1>\r\n" +
            "<a href=\"setup\">Click here to go back</a>\r\n" +
            "</body>\r\n" +
            "\r\n";
     return savedPage;
}

void handleSavedSetup() {
  maxTemp = server.arg("myText");
  Serial.println("Text received. Contents: ");
  Serial.println(maxTemp.toInt());
  EEPROM.write(addr+1, maxTemp.toInt());
  EEPROM.commit();
  Serial.println(" ");
  server.send(200, "text/html", prepareSaved());
}
