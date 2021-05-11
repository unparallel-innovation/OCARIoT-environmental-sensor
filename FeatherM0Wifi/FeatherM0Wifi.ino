/*

O codigo bem funcional

 */
//#define AIO_SERVER      "192.168.1.104"
//#define AIO_SERVERPORT  1883
//#define AIO_USERNAME    "ocariot"
//#define AIO_KEY         "ocariotkey"

//#include <Adafruit_SleepyDog.h>
//#include "Adafruit_MQTT.h"
//#include "Adafruit_MQTT_Client.h"
#include <ArduinoHttpClient.h>
#include <Wire.h>
#include <Adafruit_AM2315.h>
#include <SPI.h>
#include <WiFi101.h>
#include <ArduinoJson.h>
#include <time.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define MEASURE_PERIOD 900000 //15 min
//#define MEASURE_PERIOD 30000 //30 seg


const String OCARIOT_USERNAME = "";
const String OCARIOT_PASSWORD = "";
const String OCARIOT_INST = "";

char ssid[] = "";        // your network SSID (name)
char pass[] = "";    // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;     // the WiFi radio's status
Adafruit_AM2315 am2315;

StaticJsonDocument<2500> doc;

WiFiSSLClient wifiSsl;
//HttpClient client(wifiSsl, "api.ocariot.lst.tfo.upm.es", 443);
HttpClient ocariotClient(wifiSsl, "api.ocariot.lst.tfo.upm.es", 443);
//Adafruit_MQTT_Client mqtt(&wifi, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// You don't need to change anything below this line!
#define halt(s) { Serial.println(F( s )); while(1);  }

/****************************** Feeds ***************************************/


//Adafruit_MQTT_Publish ocariotsender = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/sensors");

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 0, 60000);

void setup() {
  WiFi.setPins(8,7,4,2); // identify pins for the wifi module
  WiFi.lowPowerMode(); // enable low power mode

  //Initialize serial and wait for port to open:
  Serial.begin(9600);
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB port only
//  }

 if (! am2315.begin()) {
     Serial.println("Sensor not found, check wiring & pullups!");
     while (1);
  }

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to WiFi network:
  connectWifi(ssid, pass);

  // you're connected now, so print out the data:
  Serial.print(F("You're connected to the network"));
  printCurrentNet();
  printWiFiData();
  timeClient.begin();

}

void loop() {
  float temperature, humidity;

  if (! am2315.readTemperatureAndHumidity(&temperature, &humidity)) {
    Serial.println(F("Failed to read data from AM2315"));
    return;
  }
  Serial.print(F("Temp *C: ")); Serial.println(temperature);
  Serial.print(F("Hum %: ")); Serial.println(humidity);

  // check wifi connection
  connectWifi(ssid, pass);
  sendToOcariot(temperature,humidity);
  delay(MEASURE_PERIOD);
}

//void sendToMqtt(float temperature, float humidity){
// MQTT_connect();
////Now sending info
//  String temp = String(temperature);
//  String humi = String(humidity);
//  timeClient.update();
// String postData = "{\n \"location\": {\n \"local\": \"outdoor\",\n \"room\": \"roof\"\n },\n \"measurements\": [\n {\n \"type\": \"temperature\",\n \"value\":" + temp + ",\n \"unit\": \"°C\"\n },\n {\n \"type\": \"humidity\",\n \"value\": " + humi + ",\n \"unit\": \"%\"\n }\n ],\n \"timestamp\":  \"" + timeClient.getFormattedDate()+ "\"\n}";
////  String postData = "{\n \"location\": {\n \"local\": \"indoor-sensor1\",\n \"room\": \"roof\"\n },\n \"measurements\": [\n {\n \"type\": \"temperature\",\n \"value\":" + temp + "},\n {\n \"type\": \"humidity\",\n \"value\": " + humi + " }\n ]""\n}";
//  // Now we can publish stuff!
//  Serial.print(F("\nSending photocell val "));
//  Serial.print(postData);
//  Serial.print("...");
//  if (! ocariotsender.publish(postData.c_str ())) {
//    Serial.println(F("Failed"));
//  } else {
//    Serial.println(F("OK!"));
//  }
//
//
//  }

void sendToOcariot(float temperature, float humidity){
  Serial.println(F("making POST request"));

  /*
   * TODO
   *
   * only execute authentication process when receive UNAUTHORIZED code
   *  - energy optimization
   */
  Serial.println(F("Starting authentication in OCARIoT..."));
  String postData = "{\n  \"username\": \"" + OCARIOT_USERNAME + "\",\n  \"password\": \"" + OCARIOT_PASSWORD + "\"\n}";
  ocariotClient.beginRequest();
  ocariotClient.post("/v1/auth/");
  ocariotClient.sendHeader("Content-Type", "application/json");
  ocariotClient.sendHeader("Content-Length", postData.length());
  ocariotClient.beginBody();
  ocariotClient.print(postData);
  ocariotClient.endRequest();

  // read the status code and body of the response
  int statusCode = ocariotClient.responseStatusCode();
  String response = ocariotClient.responseBody();
  if (statusCode != 200) {
    Serial.println(F("Authentication Error!!!"));
    Serial.print(F("Error message: "));
    Serial.println(response);
    return;
  }
  Serial.println(F("Authentication sucessfully"));
//  Serial.print(F("Status code: "));
//  Serial.println(statusCode);
//  Serial.print(F("Response: "));
//  Serial.println(response);
  DeserializationError error = deserializeJson(doc, response);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  const char* token = doc["access_token"];
//  Serial.println((String)token);
//  Serial.println(F("Wait five seconds"));

//  printCurrentNet();
  //Now sending info
//  String temp = String(temperature);
//  String humi = String(humidity);
  Serial.println(F("Sending data message to OcARioT"));
  timeClient.update();
  String postDataWithToken = "{\n \"location\": {\n \"local\": \"indoor\",\n \"room\": \"" + OCARIOT_USERNAME + "\"\n },\n \"measurements\": [\n {\n \"type\": \"temperature\",\n \"value\":" + String(temperature) + ",\n \"unit\": \"°C\"\n },\n {\n \"type\": \"humidity\",\n \"value\": " + String(humidity) + ",\n \"unit\": \"%\"\n }\n ],\n \"timestamp\":  \"" + timeClient.getFormattedDate()+ "\"\n}";
  Serial.println(postDataWithToken);
  ocariotClient.beginRequest();
  ocariotClient.post("/v1/institutions/" + OCARIOT_INST + "/environments/");
  ocariotClient.sendHeader("Authorization", "Bearer " + (String)token );
  ocariotClient.sendHeader("Content-Type", "application/json");
  ocariotClient.sendHeader("Content-Length", postDataWithToken.length());
  ocariotClient.beginBody();
  ocariotClient.print(postDataWithToken);
  ocariotClient.endRequest();

    // read the status code and body of the response
  statusCode = ocariotClient.responseStatusCode();
  response = ocariotClient.responseBody();

  if (statusCode != 201) {
    Serial.println(F("Error sending data!!!"));
    Serial.print(F("Error message: "));
    Serial.println(response);
    return;
  }
  Serial.println(F("Data sent successfuly"));
//  Serial.print(F("Status code: "));
//  Serial.println(statusCode1);
//  Serial.print(F("Response: "));
//  Serial.println(response1);
//  delay(5000);

  }

void connectWifi(char * ssid, char * pass) {
    while ( WiFi.status() != WL_CONNECTED) {
      Serial.print(F("Attempting to connect to WPA SSID: "));
      Serial.println(ssid);
      // Connect to WPA/WPA2 network:
      status = WiFi.begin(ssid, pass);
      // wait 10 seconds for connection:
      delay(10000);
  }

}

void printWiFiData() {
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print(F("IP Address: "));
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print(F("MAC address: "));
  printMacAddress(mac);

}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print(F("BSSID: "));
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print(F("signal strength (RSSI):"));
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print(F("Encryption Type:"));
  Serial.println(encryption, HEX);
  Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
//void MQTT_connect() {
//  int8_t ret;
//
//  // attempt to connect to Wifi network:
//  while (WiFi.status() != WL_CONNECTED) {
//    Serial.print("Attempting to connect to SSID: ");
//    Serial.println(ssid);
//    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
//    status = WiFi.begin(ssid, pass);
//
//    // wait 10 seconds for connection:
//    uint8_t timeout = 10;
//    while (timeout && (WiFi.status() != WL_CONNECTED)) {
//      timeout--;
//      delay(1000);
//    }
//  }
//
//  // Stop if already connected.
//  if (mqtt.connected()) {
//    return;
//  }
//
//  Serial.print("Connecting to MQTT... ");
//
//  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
//       Serial.println(mqtt.connectErrorString(ret));
//       Serial.println("Retrying MQTT connection in 5 seconds...");
//       mqtt.disconnect();
//         while (WiFi.status() != WL_CONNECTED) {
//            Serial.print("Attempting to connect to SSID: ");
//            Serial.println(ssid);
//            // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
//            status = WiFi.begin(ssid, pass);
//
//            // wait 10 seconds for connection:
//            uint8_t timeout = 10;
//            while (timeout && (WiFi.status() != WL_CONNECTED)) {
//              timeout--;
//              delay(1000);
//    }
//  };
//       delay(5000);  // wait 5 seconds
//       Watchdog.enable();
//  }
//  Serial.println("MQTT Connected!");
//}
