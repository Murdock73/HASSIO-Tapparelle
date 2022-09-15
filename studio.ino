#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

// Connection parms
#include <SSID.h>
#include <Studio.h>

// PubSubClient Settings
WiFiClient espClient;
PubSubClient client(espClient);
String switch1;
String saveswitch1 = " ";
String strTopic;
String strPayload;


// Misc variables
unsigned long timestamp; 

int studiogiu = D4; // Pin per tapparella studio giu
int studiosu = D3; // Pin per tapparella studio su
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

  if(strTopic == "HA/studio/shade"){
    switch1 = String((char*)payload);
    Serial.println(switch1);

    if(switch1 == "RESET") {
      Serial.println("reset");
      saveswitch1 = " ";
      ESP.restart();
    }
    
    if(switch1 == "STOP") {
      Serial.println("FERMA TUTTO");
      digitalWrite(studiosu, HIGH);
      digitalWrite(studiogiu, HIGH);
      delay(100);
      startstudio = 0;
    }

    if(switch1 == "GIU" && switch1 != saveswitch1) {
      Serial.println("fai scendere");
      startstudio = millis();
      digitalWrite(studiogiu, LOW); 
      digitalWrite(studiosu, HIGH);
      delay(100);
      saveswitch1 = switch1;
    }
    
    if(switch1 == "SU" && switch1 != saveswitch1) {
      Serial.println("fai salire");
      startstudio = millis();
      digitalWrite(studiosu, LOW);
      digitalWrite(studiogiu, HIGH);
      delay(100);
      saveswitch1 = switch1;
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
  delay(100);

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("esp8266-STUDIO");

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

void loop()
{
  ArduinoOTA.handle();
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
      delay(100);
      Serial.print("FINE TAPPARELLA");
    }
  }
}
