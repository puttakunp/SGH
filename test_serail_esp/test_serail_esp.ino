#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>

SoftwareSerial mySerial(4,5); //rx,tx//d2,d1

// Init WIFI Config
const char *ssid =  "XeusLab";
const char *pass =  "xeus1010";

// Init MQTT Config
#define MQTT_server "m13.cloudMQTT.com"
#define MQTT_port 18417
#define MQTT_username "trrcldyn"
#define MQTT_password "cdYwe7J5MMlr"

// MQTT & WIFI
WiFiClient espClient;
PubSubClient client(espClient);

int status = WL_IDLE_STATUS;
unsigned long lastSend;

// Init output PIN
#define LED_PIN 2

bool setup_mqtt()
{            
  if (!client.connected()) 
  {  
    Serial.print("Connecting to MQTT server ... ");
  
    if (client.connect("ESP8266Client", MQTT_username, MQTT_password)) 
    {
      Serial.println("Connected");
      client.subscribe("act");
      return true;
    } 
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
      return false;
    }
  }
  else
  {
    client.loop();
    return true; 
  }
}

bool setup_wifi(){
  if (WiFi.status() != WL_CONNECTED) 
  {
    delay(100);
    
    Serial.println();
    Serial.print("Connecting to WiFi ");
    while (WiFi.status() != WL_CONNECTED) 
    {
      WiFi.begin(ssid, pass);
      delay(500);
      Serial.print(".");
    }    
    Serial.println("connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    
    if (WiFi.status() != WL_CONNECTED) 
    {
      return false;
    }
    else
    {
      return true;
    }    
  }
  else
  {
    return true;
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  int i=0;
  String msg = "";
  while (i<length) msg += (char)payload[i++];
  Serial.println(msg);
  
  i=0;
  String str_topic = "";  
  while (i<sizeof(topic)) str_topic += (char)topic[i++];
  
  
}

void setup() {  
  Serial.begin(115200);
  while(!Serial){;}  
  Serial.println("Aloha Bababa ...");
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite( LED_PIN, LOW );

  client.setServer(MQTT_server, MQTT_port);
  client.setCallback(callback);
  
  mySerial.begin(9600);
  setup_wifi(); 
  setup_mqtt();
  
}

void loop() {
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    while ( status != WL_CONNECTED) {
      status = WiFi.begin(ssid, pass);
      delay(500);
    }
  }

  if ( !client.connected() ) {
    setup_mqtt();
  }
  
  int i = 0;
  
  if (mySerial.available()){
    i = mySerial.read();
//    Serial.println(mySerial.read());
//    Serial.println(mySerial.peek());
  }
  
  if ( millis() - lastSend > 1000 ) {
    String str = String(i);
    char *cstr = new char[str.length() + 1];
    strcpy(cstr, str.c_str());
    client.publish("conn",cstr);
    lastSend = millis();
  }
  
  client.loop();
  
}
