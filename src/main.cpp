#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <Credentials.h> // Should link to your own credentials.h file. See Credentials_sample.h in include-directory.

#define RXD2 16   // Tx grün
#define TXD2 17  // Rx weiß
#define LED_PIN 32

// Wifi
const char ssid[] = WIFI_SSID; // Define in include/Credentials.h
const char pass[] = WIFI_PASSWD; // Define in include/Credentials.h

// Variables
long unsigned int startTimestamp;
unsigned long nextCheckMillis;
String usedEnergyVal, producedEnergyVal, currPowerVal;


// NTP
WiFiUDP ntpUDP;
// if offset is needed replace with bottom line then.
// const long utcOffsetInSeconds = 3600;
// NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds); 
NTPClient timeClient(ntpUDP, "pool.ntp.org");


// MQTT
const char* mqttServer = MQTT_SERVER; // Define in include/Credentials.h
const char* mqttUser = MQTT_USER; // Define in include/Credentials.h
const char* mqttPassword = MQTT_PWD; // Define in include/Credentials.h
const int   mqttPort = 1883; 
const char* mqttPub = "smartmeter/status"; 

WiFiClient espClient;
PubSubClient client(espClient);


// Wifi procedures
void wifiInitialization(){
  
  if (WiFi.status() != WL_CONNECT_FAILED){
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    
    Serial.print("Checking WiFi ...");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");      
      if (digitalRead(LED_PIN) == LOW) {
        digitalWrite(LED_PIN, HIGH);
      } else {
        digitalWrite(LED_PIN, LOW);
      }
      delay(500);
    }
  }

  digitalWrite(LED_PIN, LOW);
  Serial.print(" connected with IP ");
  Serial.println(WiFi.localIP());   
}


// MQTT procedures
void connectToMQTTBroker(){  
  if (!client.connected()) {
    while (!client.connected()) {
      if(client.connect("ESP8266Client", mqttUser, mqttPassword)){
        Serial.println("Connected to MQTT Broker.");
      } else {
        Serial.print("MQTT connection to broker failed with state ");
        Serial.println(client.state());
        delay(2000);
      }
      delay(100);
    }
  }
}

void sendMQTTPayload(){
  connectToMQTTBroker();
  
  const int capacity = JSON_OBJECT_SIZE(8);
  StaticJsonDocument<capacity> doc;
  doc["currPower"] = currPowerVal;
  doc["usedEnergy"] = usedEnergyVal;
  doc["producedEnergy"] = producedEnergyVal;  
  
  char JSONmessageBuffer[300];
  serializeJson(doc, JSONmessageBuffer, sizeof(JSONmessageBuffer));
  
  if (client.publish(mqttPub, JSONmessageBuffer, true)){
    Serial.print("Send successfull to MQTT Broker: ");
    Serial.println(JSONmessageBuffer);
    Serial.println();
  } else {
    Serial.print("Error publishing MQTT message.");
  }
}

void mqttInitialization(){
  client.setServer(mqttServer, mqttPort); 
  connectToMQTTBroker(); 
}

void ntpTimeInitialization(){
  startTimestamp = 0; 
  timeClient.setUpdateInterval(43200); 
  timeClient.begin();
  timeClient.update();
  startTimestamp = timeClient.getEpochTime();
  while (startTimestamp < 1609502400) {
    timeClient.update();
    startTimestamp = timeClient.getEpochTime();
    Serial.println("Wait for valid NTP time.");
    delay(3000);
  }
  
  Serial.println(startTimestamp);
  }

void setup() {
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  wifiInitialization();

  mqttInitialization(); 

  // ntpTimeInitialization(); 
  
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  Serial2.begin(9600, SERIAL_7E1, RXD2, TXD2);
  Serial.println("Serial Txd is on pin: "+ String(TX));
  Serial.println("Serial Rxd is on pin: "+ String(RX));

  nextCheckMillis = 1;
}

void identify(String content) {
  Serial.println("### Start");
  Serial.println(content);
  Serial.println("### End");

  int foundSomething = 0;
  usedEnergyVal = "";
  producedEnergyVal = ""; 
  currPowerVal = "";

  if (content.indexOf("1-0:1.8.0*255(") > 0) {
    Serial.println("Used energy found: "); 
    usedEnergyVal = content.substring(content.indexOf("1-0:1.8.0*255(") + 14 , content.indexOf("1-0:1.8.0*255(") + 14 + 15);
    Serial.println(usedEnergyVal);
    foundSomething += 1;
  }

  if (content.indexOf("1-0:2.8.0*255(") > 0) {
    Serial.println("Produced energy found: "); 
    producedEnergyVal = content.substring(content.indexOf("1-0:2.8.0*255(") + 14 , content.indexOf("1-0:2.8.0*255(") + 14 + 15);
    Serial.println(producedEnergyVal);
    foundSomething += 1;
  }

  if (content.indexOf("1-0:16.7.0*255(") > 0) {
    Serial.println("Power found: "); 
    currPowerVal = content.substring(content.indexOf("1-0:16.7.0*255(") + 15, content.indexOf("1-0:16.7.0*255(") + 15 + 9);
    Serial.println(currPowerVal);
    foundSomething += 1;
  }

  if (foundSomething >= 3) {
    Serial.println("Send MQTT now.");
    digitalWrite(LED_PIN, HIGH); 
    sendMQTTPayload(); 
    delay(1000); 
    digitalWrite(LED_PIN, LOW);     
    nextCheckMillis = millis() + 30000;
  }    
}

void loop() { 
  String content = "";
  char character;

  if (millis() > nextCheckMillis) {
    while (Serial2.available()) { 
      character = Serial2.read();
      content.concat(character); 
      delay(1);            
    } 
    //Serial.print(content);     
    identify(content);      
    content = "";    
  }  
  
}