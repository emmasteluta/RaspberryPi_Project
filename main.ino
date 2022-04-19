#include <ArduinoMqttClient.h>
#include <DHT.h>
#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
  #include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
  #include <WiFi101.h>
#elif defined(ARDUINO_ESP8266_ESP12)
  #include <ESP8266WiFi.h>
#endif

//#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = "HAMKvisitor";        // your network SSID (name)
char pass[] = "hamkvisitor";    // your network password (use for WPA, or use as key for WEP)

int ledPin = 13;                // LED 
int pirPin = 2;                 // PIR Out pin 
int pirStat = 0;                   // PIR status

// To connect with SSL/TLS:
// 1) Change WiFiClient to WiFiSSLClient.
// 2) Change port value from 1883 to 8883.
// 3) Change broker value to a server with a known SSL/TLS root certificate 
//    flashed in the WiFi module.

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "iot.research.hamk.fi";
int        port     = 1883;
const char topic[]  = "HAMK/VLK/students/306";

const long interval = 10000;
unsigned long previousMillis = 0;

int count = 0;
float fTemp, fHum, light_val;
int t_attempt;
int h_attempt;

//sensor def
#define DHT11Pin 2
#define TEMPTYPE 0 
DHT dht(DHT11Pin, DHT11);

void setup() {
  //Initialize serial and wait for port to open:
  pinMode(ledPin, OUTPUT);     
  pinMode(pirPin, INPUT);     
 
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  Serial.println();

  // You can provide a unique client ID, if not set the library uses Arduino-millis()
  // Each client must have a unique client ID
  // mqttClient.setId("clientId");

  // You can provide a username and password for authentication
  // mqttClient.setUsernamePassword("username", "password");

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
  dht.begin();
}

void loop() {
  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  mqttClient.poll();

  // avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
  // see: File -> Examples -> 02.Digital -> BlinkWithoutDelay for more info
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;
    
    fTemp = dht.readTemperature();
    fHum = dht.readHumidity();
    light_val=analogRead(0);
    String pirStatMsg = "";

    pirStat = digitalRead(pirPin); 
    if (pirStat == HIGH) {            // if motion detected
    //digitalWrite(ledPin, HIGH);  // turn LED ON
    pirStatMsg = "Motion detected";
    } 
    else {
    //digitalWrite(ledPin, LOW); // turn LED OFF if we have no motion
    pirStatMsg = "No motion";
    }

    
    Serial.print("Sending message to topic: ");
    Serial.println(topic);
    Serial.print(fTemp);
    Serial.print(";");
    Serial.println(fHum);
    Serial.println(pirStat);

    
    mqttClient.beginMessage(topic);
    mqttClient.print("{\"temperature\": ");
    mqttClient.print(fTemp);
    
    mqttClient.print(",\"humidity\": ");
    mqttClient.print(fHum);
    mqttClient.print(", \"light\": ");
    mqttClient.print(light_val);
    mqttClient.print(", \"motion status\": ");
    mqttClient.print(pirStat);
    mqttClient.print("}");
    mqttClient.endMessage();
    

    Serial.println();

  }
}
