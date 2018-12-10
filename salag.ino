#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Connection parms
const char* ssid = "***********";
const char* password = "***********";
const char* mqtt_server = "192.168.1.xxx";
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

int salaggiu = 0; // Pin per tapparella salag giu
int salagsu = 2; // Pin per tapparella salag su
unsigned long startsalag = 0;
unsigned long endsalag = 31000;
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

  if(strTopic == "HA/salag/shade"){
    switch1 = String((char*)payload)
       
    if(switch1 == "STOP") {
      Serial.println("STOP");
      startcucina = millis();
      digitalWrite(cucinasu, HIGH);
      digitalWrite(cucinagiu, HIGH); 
    }
    
    if(switch1 == "GIU") {
      Serial.println("GIU");
      if (startsalag > 0) {         //se il contatore del tempo è > 0 stoppo i relè
        digitalWrite(salaggiu, HIGH);
        digitalWrite(salagsu, HIGH);      
        startsalag = 0;
        Serial.print("FINE TAPPARELLA");
      } else {
        startsalag = millis();
        digitalWrite(salaggiu, LOW); 
      }
    }
    
    if(switch1 == "SU") {
      Serial.println("SU");
      if (startsalag > 0) {         //se il contatore del tempo è > 0 stoppo i relè
        digitalWrite(salaggiu, HIGH);
        digitalWrite(salagsu, HIGH);      
        startsalag = 0;
        Serial.print("FINE TAPPARELLA");
      } else {
        startsalag = millis();
        digitalWrite(salagsu, LOW); 
      }
    }
  }
}
 
 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266_salag",MQTTuser, MQTTpwd)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.subscribe("HA/salag/#");
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
  pinMode(salaggiu, OUTPUT);
  digitalWrite(salaggiu, HIGH);
  pinMode(salagsu, OUTPUT);
  digitalWrite(salagsu, HIGH);

}
 
void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  if (startsalag > 0) {
    if ((millis() - startsalag) > endsalag 
     || (millis() - startsalag) < 0) {
      digitalWrite(salaggiu, HIGH);
      digitalWrite(salagsu, HIGH);      
      startsalag = 0;
      Serial.print("FINE TAPPARELLA");
    }
  }
}
