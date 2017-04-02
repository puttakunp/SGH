#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

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

// Init output PIN
#define LED_PIN 2

// Init I2C
#define I2C_SCL_PIN      (5)      // D1 pin (SCL / GPIO-5)
#define I2C_SDA_PIN      (4)      // D2 pin (SDA / GPIO-4)
#define I2C_ADDR      (0x22)      // i2c address of the slave device
char sbuf[64];

//Init Serial
byte tBuf[7] = {0, 0, 0, 0, 0, 0, 0}; 
byte recev[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
bool check = false;

long lastSend = 0;
long lastRec = 0;
long lastConn = 0;
long timeout = 0;
long now = 0;
long d;

// init sensor_val
int Light_Int_val = 0;
int BH1750_address = 0x23; // i2c Addresse
byte buff_BH1750[2];

void(* resetFunc) (void) = 0;



void Send_State(  uint8_t i2c_addr,  uint8_t len )  
{  
    Wire.beginTransmission(I2C_ADDR); 
    Wire.write(0);
    Wire.write(tBuf,len);
    Wire.endTransmission();
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
  
  digitalWrite(LED_PIN, (msg == "LEDON" ? HIGH : LOW));

//  if (msg == "GET") {
//    client.publish("/ESP/LED", (digitalRead(LED_PIN) ? "LEDON" : "LEDOFF"));
//    Serial.println("Send !");
//    return;
//  }
  
  if (msg == "10RESET01") {    
    delay(1500);
    resetFunc();
    return;
  }
  
  if(str_topic == "act")
  {     
    tBuf[1] = payload[0]-48 ; 
    tBuf[2] = payload[1]-48 ;
    tBuf[3] = payload[2]-48 ; 
    tBuf[4] = payload[3]-48 ;
    tBuf[5] = payload[4]-48 ; 
    
    Serial.println(); 
    Serial.print("Send State : ");
    Serial.print(tBuf[1]);
    Serial.print(" : ");
    Serial.print(tBuf[2]);
    Serial.print(" : ");   
    Serial.print(tBuf[3]);
    Serial.print(" : ");
    Serial.print(tBuf[4]);
    Serial.print(" : ");
    Serial.println(tBuf[5]);
    Serial.println(); 

    Send_State( I2C_ADDR, 7 ); 
    lastSend = millis() - 7000;     
  }

  if (msg == "99999") {
    delay(1000);
    client.publish("act", "00000");
  }

}




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
    WiFi.begin(ssid, pass);
    Serial.println();
    Serial.print("Connecting to WiFi ");
    while (WiFi.status() != WL_CONNECTED) 
    {
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

void i2c_scan() {
  int count = 0;
  Serial.println( "Scanning I2C slave devices..." );
  for( uint8_t addr=0x01; addr <= 0x7f; addr++ ) {
     Wire.beginTransmission( addr ); 
     if ( Wire.endTransmission() == 0 ) {
       sprintf( sbuf, "I2C device found at 0x%02X.", addr );
       Serial.println( sbuf );
       count++;
    }
  }
  if (count > 0 ) {
    sprintf( sbuf, "Found %d I2C devices.", count );
  } else {
    sprintf( sbuf, "No I2C device found." );
  }
  Serial.println( sbuf );
}



//////////////////////////////////// Receive_Val  ////////////////////////////////////

void Receive_Val(uint8_t i2c_addr, uint8_t len ) 
{
  uint8_t buf[15];  
  delayMicroseconds(100);  
  Wire.requestFrom( (int)i2c_addr, (int)len );
  timeout = millis(); 
  while ( Wire.available() < len )    // waiting  
  {
    now = millis();                     
    if (now - timeout > 5000)         //time out 5 second
    { 
      break;    
    }
  }
  
  uint8_t index=0;
  for ( int i=0; i < len; i++ ) 
  {
    buf[index++] = Wire.read();
  }
  Wire.endTransmission();


  int chkNULL = 0;
  String object = "{";
  for(int j=1;j<=9;j++)
  {
    if(j==1)
    {
      d = (long)Light_Int_val;
    }
    else if(j==2)
    {
      byte b[4]= {0, 0, buf[11], buf[j]};
      d = bytesToInteger(b);
    }
    else if(j==3)
    {
      byte b[4]= {0, 0, buf[12], buf[j]};
      d = bytesToInteger(b);
    }
    else if(j==4)
    {
      byte b[4]= {0, 0, buf[13], buf[j]};
      d = bytesToInteger(b);
    }
    else
    {
      d = buf[j];
    }
    
    chkNULL = chkNULL + d;
    char msg[30];
    snprintf (msg, 30, "\"%ld\":%ld",j,d);
    object.concat(msg);      
    if(j!=9){
      object.concat(",");
    }                                                                                                                                                    
  }

  object.concat("}");
  char *cstr = new char[object.length() + 1];
  strcpy(cstr, object.c_str());
  if(chkNULL != 0)
  {          
    client.publish("val", cstr);     
    Serial.println(object);
  }
  else
  {
    Serial.println("checkNULL = 0;");
    delay(1000);
  }
}


////////////////////////////////////  setup  ////////////////////////////////////

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("Start [I] ...");  

  pinMode(LED_PIN, OUTPUT);
  digitalWrite( LED_PIN, LOW );
 
  tBuf[0] = 173 ;   
  tBuf[1] = 0 ;   
  tBuf[2] = 0 ;  
  tBuf[3] = 0 ;   
  tBuf[4] = 0 ; 
  tBuf[5] = 0 ;   
  tBuf[6] = 237 ;  
  
  client.setServer(MQTT_server, MQTT_port);
  client.setCallback(callback);

  setup_wifi();  
  setup_mqtt();
  
  Serial.flush();
  Wire.begin( I2C_SDA_PIN, I2C_SCL_PIN ); 
  Wire.setClockStretchLimit(15000);
  
  i2c_scan();
  delay(1000);
}


////////////////////////////////////  loop  ////////////////////////////////////

void loop() {

  
  delay(1);   
  if(setup_wifi()) {     
    if(setup_mqtt()) {
      
      // sent state esp connection every 5 second
      now = millis();                     
      if (now - lastConn> 5000) { 
        lastConn = now;
        
        client.publish("conn", "1");        
      }


      // Receive and Send Value from Arduino to MQTT every 1 second
      now = millis();   
      if (now - lastRec > 1500 ) {
        lastRec = now;  

        Serial.print("."); 
        Wire.beginTransmission(I2C_ADDR); 
         if ( Wire.endTransmission() == 0 ) {
           Receive_Val( I2C_ADDR, 15 );
           delay(500);
           Send_State( I2C_ADDR, 7 );
           delay(500);
            // Sent State to Arduino I2C every 10 second
//            now = millis(); 
//            if (now - lastSend > 10) {      
//              lastSend = now;
//              
//              Send_State( I2C_ADDR, 7 );               
//            } 
           
        }
      }

           
      
    }    
  }
    
  
}












