#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>

SoftwareSerial mySerial(4, 5); //rx,tx//d2,d1

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

// Init Serial
byte tBuf[8];
byte rBuf[28];

const int sensor_Count = 10;
long sensor_val[sensor_Count];

const int act_Count = 6;
uint8_t act_state[act_Count];
long act_read[act_Count];



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

bool setup_wifi() {
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

  int i = 0;
  String msg = "";
  while (i < length) msg += (char)payload[i++];
  Serial.println(msg);

  i = 0;
  String str_topic = "";
  while (i < sizeof(topic)) str_topic += (char)topic[i++];


}

long bytesToInteger(byte b[4]) {
  long val = 0;
  val = ((long )b[0]) << 24;
  val |= ((long )b[1]) << 16;
  val |= ((long )b[2]) << 8;
  val |= b[3];
  return val;
}

void integerToBytes(long val, byte b[4]) {
  b[0] = (byte )((val >> 24) & 0xff);
  b[1] = (byte )((val >> 16) & 0xff);
  b[2] = (byte )((val >> 8) & 0xff);
  b[3] = (byte )(val & 0xff);
}

//////////////////////////////////// setup  ////////////////////////////////////

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  Serial.println("Aloha Bababa ---------------------------------");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite( LED_PIN, LOW );

  client.setServer(MQTT_server, MQTT_port);
  client.setCallback(callback);

  setup_wifi();
  setup_mqtt();

  mySerial.begin(112500);
  while (mySerial.available()) {
    char ch = mySerial.read();
  }

  for (int i = 0; i < sizeof(rBuf); i++)
  {
    rBuf[i] = 0;
  }

}


////////////////////////////////////  loop  ////////////////////////////////////
void loop() {
  ///////////// check connect Wifi /////////////
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    while ( status != WL_CONNECTED) {
      status = WiFi.begin(ssid, pass);
      delay(500);
    }
  }
  else
  {
    ///////////// check connect MQTT /////////////
    if ( !client.connected() ) {
      setup_mqtt();
    }
  }

  ///////////// Receive value for Arduino /////////////

  int k = 0;
  bool check = false;
  while (mySerial.available())
  {
    rBuf[k] = mySerial.read();

    if (rBuf[k] == 173 && k == 0)
    {
      check = true;
    }

    if (check == true)
    {
      if (rBuf[k] == 237 && k == 27)
      {
        k = 0;
        check = false;
        break;
      }
      k++;
    }
  }

  //    // print rBuf
  //    for (int i = 0; i < sizeof(rBuf); i++)
  //    {
  //      Serial.print(i);
  //      Serial.print(" : ");
  //      Serial.println(rBuf[i]);
  //    }

  ///////////// !Receive rBuf /////////////





  if ( millis() - lastSend > 1000 ) {

    int chkSum = 0;
    for (int i = 0; i < sizeof(rBuf); i++)
    {
      chkSum = chkSum + rBuf[i];
    }




    ///////////// rBuf to Json /////////////

    char msg[30];

    // act
    int j = 0;
    String object1 = "{";
    for (int i = 0; i < act_Count; i++)
    {
      act_read[i] = rBuf[i + 1]  ;
      //      Serial.print(act_read[i]);

      snprintf (msg, 30, "\"%ld\":%ld", j, act_read[i]);
      object1.concat(msg);
      if (i < act_Count - 1)
      {
        object1.concat(",");
      }
      j++;
    }
    object1.concat("}");


    // val
    j = 0;
    String object2 = "{";
    for (int i = 0; i < sensor_Count; i++)
    {
      byte bVal[4] = {0, 0, rBuf[i + act_Count + sensor_Count + 1], rBuf[i + act_Count + 1]};
      sensor_val[i] = bytesToInteger(bVal);
      //      Serial.println(sensor_val[i]);

      snprintf (msg, 30, "\"%ld\":%ld", j, sensor_val[i]);
      object2.concat(msg);
      if (i < sensor_Count - 1)
      {
        object2.concat(",");
      }
      j++;
    }
    object2.concat("}");

    char *cstr1 = new char[object1.length() + 1];
    strcpy(cstr1, object1.c_str());

    char *cstr2 = new char[object2.length() + 1];
    strcpy(cstr2, object2.c_str());

    Serial.println(cstr1);
    Serial.println(cstr2);
    
    ///////////// ! rBuf to Json /////////////


    ///////////// Sent rBuf to MQTT /////////////

    if (chkSum != 0)
    {
      client.publish("act_state", cstr1);
      client.publish("val_sensor", cstr2);
      Serial.println("act_state,val_sensor ---->  mqtt");      
    }
    else
    {
      Serial.println("checkNULL = 0;");
    }
    ///////////// ! Sent rBuf to MQTT /////////////




    // -------------- code here --------------




    Serial.println(".");
    client.publish("conn", "1");
    lastSend = millis();
    Serial.println("--------------");

  }
  client.loop();

}
