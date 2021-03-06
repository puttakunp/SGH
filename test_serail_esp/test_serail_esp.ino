#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>
#include <FirebaseArduino.h>

SoftwareSerial mySerial(4, 5); //rx,tx//d2,d1

// Init WIFI Config
const char *ssid =  "XeusLab";
const char *pass =  "xeus1010";

// Init MQTT Config
#define MQTT_server "m13.cloudMQTT.com"
#define MQTT_port 18417
#define MQTT_username "trrcldyn"
#define MQTT_password "cdYwe7J5MMlr"

// Init Firebase Config
String  FIREBASE_HOST = "greenhousenu-f3322.firebaseio.com";
const String FIREBASE_KEY = "z724Hk0z5oYKRkC81EsZdshMLkTAPNt8hoLasjTV";

// MQTT & WIFI
WiFiClient espClient;
PubSubClient client(espClient);

int status = WL_IDLE_STATUS;
unsigned long lastSend;

// Init output PIN
#define LED_PIN 2
#define LED_R 14
#define LED_G 12
#define LED_B 13

// Init Serial
byte tBuf[8];
byte rBuf[29];

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

      digitalWrite(LED_G, HIGH);
      digitalWrite(LED_B, HIGH);
      digitalWrite(LED_R, LOW);
      return true;
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);

      digitalWrite(LED_G, HIGH);
      digitalWrite(LED_B, LOW);
      digitalWrite(LED_R, LOW);
      return false;
    }
  }
  else
  {
    client.loop();
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, HIGH);
    digitalWrite(LED_R, LOW);
    return true;
  }
}




bool setup_wifi() {
  if (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_B, LOW);
    digitalWrite(LED_G, LOW);
    delay(100);

    Serial.println();
    Serial.print("Connecting to WiFi ");
    int count = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(ssid, pass);
      delay(500);
      Serial.print(".");
      count++;
      if (count > 20)
      {
        break;
      }
    }
    Serial.println("connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    if (WiFi.status() != WL_CONNECTED)
    {
      digitalWrite(LED_R, HIGH);
      digitalWrite(LED_B, LOW);
      digitalWrite(LED_G, LOW);
      return false;

    }
    else
    {

      digitalWrite(LED_R, LOW);
      digitalWrite(LED_G, HIGH);
      digitalWrite(LED_B, LOW);
      return true;
    }
  }
  else
  {
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, LOW);
    return true;
  }
}




void callback(char* topic, byte* payload, unsigned int length)
{
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


  ///////////// Sent tBuf to Arduino /////////////

  Serial.println(length);
  Serial.println(sizeof(tBuf) - 2);

  if (length == sizeof(tBuf) - 2)
  {

    Serial.print("! Sent tBuf to Arduino : ");
    for (int i = 0; i < sizeof(tBuf) - 2 ; i++)
    {
      tBuf[i+1] = payload[i] - '0';

      Serial.print(tBuf[i+1]);
    }
    mySerial.write(tBuf, sizeof(tBuf));

    Serial.println("  OK");
    Serial.println();

  }

  ///////////// !Sent tBuf to Arduino /////////////

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


  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  digitalWrite(LED_R, LOW);    // off
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);
  digitalWrite(LED_PIN, LOW);  // on

  client.setServer(MQTT_server, MQTT_port);
  client.setCallback(callback);

  setup_wifi();
  delay(1000);
  setup_mqtt();

  Firebase.begin(FIREBASE_HOST, FIREBASE_KEY);

  tBuf[0] = 173 ; // byte_start
  tBuf[1] = 0 ; // Led
  tBuf[2] = 0 ; // Watering
  tBuf[3] = 0 ; // Fogger
  tBuf[4] = 0 ; // Evap_cool
  tBuf[5] = 0 ; // Vent_Fan
  tBuf[6] = 0 ; // Vent_Fan
  tBuf[7] = 237 ; // byte_stop

  mySerial.begin(112500);
  while (mySerial.available()) {
    char ch = mySerial.read();
  }
}

////////////////////////////////////  loop  ////////////////////////////////////
void loop() {
  client.loop();

  ///////////// check connect Wifi /////////////
  status = WiFi.status();
  if ( status != WL_CONNECTED)
  {
    setup_wifi();
  }
  else
  {
    ///////////// check connect MQTT /////////////
    if ( !client.connected() ) {
      setup_mqtt();
    }
    else
    {
      if ( millis() - lastSend > 0 )
      {
        lastSend = millis();




        ///////////// Receive value for Arduino /////////////

        //set rBuf 0
        for (int i = 0; i < sizeof(rBuf); i++)
        {
          rBuf[i] = 0;
        }

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
            if (rBuf[k] == 237 && k == (sizeof(rBuf) - 1))
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


        int chkSum = 0;
        for (int i = 0; i < sizeof(rBuf); i++)
        {
          chkSum = chkSum + rBuf[i];
        }

        Serial.println(chkSum);
        if (chkSum != 0 && chkSum > 300)
        {
          digitalWrite(LED_R, HIGH);


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
          snprintf (msg, 30, ",\"%ld\":%ld", j, int(rBuf[sizeof(rBuf) - 2]));
          object1.concat(msg);
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


          /////////////            Sent Data          /////////////

          ///////////// Sent Data to MQTT /////////////
          Serial.print("act_state,val_sensor ---->  mqtt ...");
          client.publish("act_state", cstr1);
          client.publish("val_sensor", cstr2);
          Serial.println("ok");

          ///////////// Sent Data to firebase /////////////
          Serial.print("act_state,val_sensor ---->  firebase ...");
          StaticJsonBuffer<200> jsonBuffer;
          JsonObject& root = jsonBuffer.parseObject(cstr1);
          if (!root.success()) {
            Serial.println("parseObject() failed");
            return;
          }
          Firebase.set("act_state", root );

          StaticJsonBuffer<200> jsonBuffer2;
          JsonObject& root2 = jsonBuffer2.parseObject(cstr2);
          if (!root.success()) {
            Serial.println("parseObject() failed");
            return;
          }
          Firebase.set("val_sensor", root2 );
          Serial.println("ok");
          ///////////// ! Sent Data to firebase /////////////


          digitalWrite(LED_R, LOW);


          //clear buffer rBuf
          while (mySerial.available())
          {
            char ch = mySerial.read();
          }


        }
        else
        {
          Serial.println("checkNULL = 0;");
        }

        // -------------- code here --------------




        Serial.println(".");
        client.publish("conn", "1");
        Serial.println("--------------");

      }
    }
  }
}
