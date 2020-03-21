#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h> 
#include <DallasTemperature.h>

// Setup for Onewire Temp sensor
// Data wire is plugged into pin 2 on the ESP8266 
#define ONE_WIRE_BUS D4
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature DS18B20(&oneWire);

// Connection parms
const char* ssid = "**********";
const char* password = "**********";
const char* mqtt_server = "192.168.1.***";
const char* MQTTuser = "**********";
const char* MQTTpwd = "**********";


// PubSubClient Settings
WiFiClient espClient;
PubSubClient client(espClient);
String switch1;
String saveswitch1 = " ";
String strTopic;
String strPayload;
#define temperature_topic "HA/salap/temperature/state"

// Misc variables
unsigned long timestamp; 
const int PubInterval = 60000; // Intervallo per pubblicazione e lettura sensore in ms
float temp = 0.0;
String temperature;
int terrazzo = D0; // Pin per luci terrazzo
int giardino = D1; // Pin per luci giardino
int salapgiu = D2; // Pin per tapparella sala giu
int salapsu = D3; // Pin per tapparella sala giu
unsigned long startsalap = 0;
unsigned long endsalap = 24000;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  timestamp = millis();
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  strTopic = String((char*)topic);
  Serial.println(strTopic);

 if(strTopic == "HA/salap/garden"){
    switch1 = String((char*)payload);
    if(switch1 == "ON") {
      Serial.println("ON");
      digitalWrite(giardino, LOW);
    }
    else {
      Serial.println("OFF");
      digitalWrite(giardino, HIGH); 
    }
  }
    
  if(strTopic == "HA/salap/terrace"){
    switch1 = String((char*)payload);
    if(switch1 == "ON") {
      Serial.println("ON");
      digitalWrite(terrazzo, LOW); 
    }
    else {
      Serial.println("OFF");
      digitalWrite(terrazzo, HIGH); 
    }
  }

    
  if(strTopic == "HA/salap/shade") {
    switch1 = String((char*)payload);
    
    
    if(switch1 == "RESET") {
      Serial.println("reset");
      saveswitch1 = " ";
      ESP.restart();
    }
    
    if(switch1 == "STOP") {
      Serial.println("FERMA TUTTO");
      digitalWrite(salapsu, LOW);
      digitalWrite(salapgiu, LOW);
      digitalWrite(salapsu, HIGH);
      digitalWrite(salapgiu, HIGH);
      startsalap = 0;
    }

    if(switch1 == "GIU" && switch1 != saveswitch1) {
      Serial.println("fai scendere");
      startsalap = millis();
      digitalWrite(salapgiu, LOW); 
      digitalWrite(salapsu, HIGH); 
      saveswitch1 = switch1;
    }
    
    if(switch1 == "SU" && switch1 != saveswitch1) {
      Serial.println("fai salire");
      startsalap = millis();
      digitalWrite(salapsu, LOW);
      digitalWrite(salapgiu, HIGH);
      saveswitch1 = switch1;
    }
  }
}
 
 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266_salap",MQTTuser, MQTTpwd)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.subscribe("HA/salap/#");
     } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
 
void setup()
{
  Serial.begin(115200);
  setup_wifi(); 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.println("Dallas Temperature IC Control Library Demo"); 
// Start up the library 
  DS18B20.begin(); 
  pinMode(giardino, OUTPUT);
  digitalWrite(giardino, HIGH);
  pinMode(terrazzo, OUTPUT);
  digitalWrite(terrazzo, HIGH);
  pinMode(salapgiu, OUTPUT);
  digitalWrite(salapgiu, HIGH);
  pinMode(salapsu, OUTPUT);
  digitalWrite(salapsu, HIGH);

}
 
void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  if ((millis() - timestamp) > PubInterval) {

    timestamp = millis();
    
    Serial.print(" Requesting temperatures..."); 
    DS18B20.requestTemperatures(); // Send the command to get temperature readings 
    Serial.println("DONE"); 
    temp = DS18B20.getTempCByIndex(0);
    Serial.print("Temperature is: "); 
    Serial.println(temp);  
    
    // pubblico la temperatura sul sensore MQTT
    temp = DS18B20.getTempCByIndex(0);
    Serial.print("New temperature:");
    Serial.println(String(temp).c_str());
    client.publish(temperature_topic, String(temp).c_str(), true);

  }

  if (startsalap > 0) {
    if ((millis() - startsalap) > endsalap 
     || (millis() - startsalap) < 0) {
      digitalWrite(salapgiu, HIGH);
      digitalWrite(salapsu, HIGH);      
      startsalap = 0;
      Serial.print("FINE TAPPARELLA");
    }
  }


}
