#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Connection parms
const char* ssid = "**********";
const char* password = "***********";
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

int cucinagiu = 0; // Pin per tapparella cucina giu
int cucinasu = 2; // Pin per tapparella cucina su
unsigned long startcucina = 0;
unsigned long endcucina = 26000;
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

  
  if (firstshot = false) {
    firstshot = true;
  } else {
    if(strTopic == "HA/cucina/shade"){
      switch1 = String((char*)payload);
      
      if(switch1 == "STOP") {
        Serial.println("STOP");
        startcucina = millis();
        digitalWrite(cucinasu, HIGH);
        digitalWrite(cucinagiu, HIGH); 
      }
      
      if(switch1 == "GIU") {
        Serial.println("GIU");
        startcucina = millis();
        digitalWrite(cucinasu, HIGH); // in caso di reverse del comando spengo il rele inverso
        digitalWrite(cucinagiu, LOW); 
      }
    
      if(switch1 == "SU") {
        Serial.println("SU");
        startcucina = millis();
        digitalWrite(cucinagiu, HIGH); // in caso di reverse del comando spengo il rele inverso
        digitalWrite(cucinasu, LOW); 
      }
    }
  }
}
 
 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266_cucina",MQTTuser, MQTTpwd)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.subscribe("HA/cucina/#");
      client.subscribe("HA/stop/#");
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
  pinMode(cucinagiu, OUTPUT);
  digitalWrite(cucinagiu, HIGH);
  pinMode(cucinasu, OUTPUT);
  digitalWrite(cucinasu, HIGH);

}
 
void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  if (startcucina > 0) {
    if ((millis() - startcucina) > endcucina 
     || (millis() - startcucina) < 0) {
      digitalWrite(cucinagiu, HIGH);
      digitalWrite(cucinasu, HIGH);      
      startcucina = 0;
      Serial.print("FINE TAPPARELLA");
    }
  }
}
