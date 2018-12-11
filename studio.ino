#include <ESP8266WiFi.h>
#include <PubSubClient.h>

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
String strTopic;
String strPayload;


// Misc variables
unsigned long timestamp; 

int studiogiu = 0; // Pin per tapparella studio giu
int studiosu = 2; // Pin per tapparella studio su
unsigned long startstudio = 0;
unsigned long endstudio = 23000;
bool firstshot = false;

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

  if(strTopic == "HA/studio/shade"){
    switch1 = String((char*)payload);
      
    if(switch1 == "STOP") {
      Serial.println("STOP");
      startstudio = millis();
      digitalWrite(studiosu, HIGH);
      digitalWrite(studiogiu, HIGH); 
    }
  
    if(switch1 == "GIU") {
      Serial.println("GIU");
      if (startstudio > 0) {         //se il contatore del tempo è > 0 stoppo i relè
        digitalWrite(studiogiu, HIGH);
        digitalWrite(studiosu, HIGH);      
        startstudio = 0;
        Serial.print("FINE TAPPARELLA");
      } else {
        startstudio = millis();
        digitalWrite(studiogiu, LOW); 
      }
    }
    
    if(switch1 == "SU") {
      Serial.println("SU");
      if (startstudio > 0) {         //se il contatore del tempo è > 0 stoppo i relè
        digitalWrite(studiogiu, HIGH);
        digitalWrite(studiosu, HIGH);      
        startstudio = 0;
        Serial.print("FINE TAPPARELLA");
      } else {
        startstudio = millis();
        digitalWrite(studiosu, LOW); 
      }
    }
  }
}
 
 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266_studio", MQTTuser, MQTTpwd)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.subscribe("HA/studio/#");
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
  pinMode(studiogiu, OUTPUT);
  digitalWrite(studiogiu, HIGH);
  pinMode(studiosu, OUTPUT);
  digitalWrite(studiosu, HIGH);

}
 
void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  if (startstudio > 0) {
    if ((millis() - startstudio) > endstudio 
     || (millis() - startstudio) < 0) {
      digitalWrite(studiogiu, HIGH);
      digitalWrite(studiosu, HIGH);      
      startstudio = 0;
      Serial.print("FINE TAPPARELLA");
      client.publish(studio_topic, "STOP"); // cambio lo stato dello switch MQTT a STOP
    }
  }
}
