#include <Wire.h>
#include "DHT.h"
#include <EEPROM.h>
#include <avr/wdt.h>

// init Pin
DHT dht;
int RGBPin[] = { 9, 10, 11 };
int BH1750_address = 0x23; // i2c Addresse
int SoilPin = A3;

int Led_0_pin = 9;
int Watering_1_pin = 10;
int Vent_Fan_2_pin = 11;
int Evap_cool_3_pin = 12;
int Fogger_4_pin = 13;


// init sensor_val
int Light_Int_val = 0;
int Soil_Hum_val = 0;
int Air_RH_val = 0;
int Air_Temp_val = 0;

byte buff_BH1750[2];

// init act_state

bool Led_state;
bool Watering_state;
bool Vent_Fan_state;
bool Evap_cool_state;
bool Fogger_state;

uint8_t Led;
uint8_t Watering;
uint8_t Vent_Fan;
uint8_t Evap_cool;
uint8_t Fogger_ki;

// init I2C
#define I2C_ADDR             (0x22)

byte tBuf[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
byte recev[7] = {0, 0, 0, 0, 0, 0, 0};
bool checkRec = 0;
uint8_t Sum_Reset ;
uint8_t act_type ;
uint8_t act_state ;
long lastSend = 0;
long now = 0;


#define resetFunc()         \  
do                          \
{                           \
    wdt_enable(WDTO_15MS);  \
    for(;;)                 \
    {                       \
    }                       \
} while(0)


void i2c_response()
{   
  Wire.write(tBuf,sizeof(tBuf));  
  wdt_reset();
}


//////////////////////////////////// GetStateValue  ////////////////////////////////////
void GetStateValue()
{  
  noInterrupts();
  Soil_Hum_val = 0;
  Air_RH_val = 0;
  Air_Temp_val = 0;
  Light_Int_val = 0;
  interrupts();
  //INPUT : Sensors : 
  
  // if(BH1750_Read(BH1750_address)==2){
  //   float valf=0;
  //   valf = ((buff_BH1750[0]<<8)|buff_BH1750[1])/1.2;    
  //   if(valf<0)
  //   {
  //     Light_Int_val = 65535;
  //   }
  //   else 
  //   {
  //     Light_Int_val = valf+50;
  //   }       
  // }  
  noInterrupts();
  delay(dht.getMinimumSamplingPeriod());
  float Air_RH_fl = dht.getHumidity();
  float Air_Temp_fl = dht.getTemperature();

  Air_RH_val = Air_RH_fl*10;
  Air_Temp_val = Air_Temp_fl*10;    

  Soil_Hum_val = analogRead(SoilPin);
  interrupts();
 
//  Serial.println(); 
//  Serial.print("Light_Int_val \t: ");
//  Serial.print(Light_Int_val,DEC);  
//  Serial.println(" lx"); 
//  Serial.print("Air_RH_val \t: ");
//  Serial.print(Air_RH_val, 1);
//  Serial.println(" %RH");
//  Serial.print("Air_Temp_val \t: ");
//  Serial.print(Air_Temp_val, 1);
//  Serial.println(" C");
//  Serial.print("Soil_Air_RH_val : " );
//  Serial.println(Soil_Hum_val);  
//  Serial.println("________________________");
//  Serial.println();
  
  
  // : Actuators State :
  noInterrupts();
  Led_state = (Led == HIGH) ? 1 : 0 ;
  Watering_state = (Watering == HIGH) ? 0 : 1 ;
  Vent_Fan_state = (Vent_Fan == HIGH) ? 0 : 1 ;
  Evap_cool_state = (Evap_cool == HIGH) ? 0 : 1 ;
  Fogger_state = (Fogger_ki == HIGH) ? 0 : 1 ;
  
//  Serial.print("Led \t\t: " );
//  Serial.println(Led_state);
//  Serial.print("Watering \t: " );
//  Serial.println(Watering_state);
//  Serial.print("Vent_Fan \t: " );
//  Serial.println(Vent_Fan_state);
//  Serial.print("Evap_cool \t: " );
//  Serial.println(Evap_cool_state);
//  Serial.print("Fogger \t\t: " );
//  Serial.println(Fogger_state);
//  
//  Serial.println("________________________");
//  Serial.println();


  byte light[4];
  integerToBytes(Light_Int_val, light);
  for (int i=0; i<4; ++i) {   
  }
  tBuf[1] = light[3];  
  tBuf[10] = (int)light[2];

  byte Air_RH[4];
  integerToBytes(Air_RH_val, Air_RH);
  for (int i=0; i<4; ++i) {   
  }
  tBuf[2] = Air_RH[3];  
  tBuf[11] = (int)Air_RH[2];

  byte Air_Temp[4];
  integerToBytes(Air_Temp_val, Air_Temp);
  for (int i=0; i<4; ++i) {   
  }
  tBuf[3] = Air_Temp[3];  
  tBuf[12] = (int)Air_Temp[2];

  byte Soil_Hum[4];
  integerToBytes(Soil_Hum_val, Soil_Hum);
  for (int i=0; i<4; ++i) {   
  }
  tBuf[4] = Soil_Hum[3];  
  tBuf[13] = (int)Soil_Hum[2];

  tBuf[9] = Fogger_state ;
  tBuf[8] = Evap_cool_state ;
  tBuf[7] = Vent_Fan_state ;
  tBuf[6] = Watering_state ;
  tBuf[5] = Led_state ;
  interrupts();
//  Serial.println(tBuf[0]);
//  Serial.println(tBuf[1]);
//  Serial.println(tBuf[2]);
//  Serial.println(tBuf[3]);
//  Serial.println(tBuf[4]);
//  Serial.println(tBuf[5]);
//  Serial.println(tBuf[6]);
//  Serial.println(tBuf[7]);
//  Serial.println(tBuf[8]);
//  Serial.println(tBuf[9]);
//  Serial.println(tBuf[10]);
//  Serial.println(tBuf[11]);
//  Serial.println(tBuf[12]);
//  Serial.println(tBuf[13]);
//  Serial.println(tBuf[14]);
//  
//  Serial.println("________________________");
//  Serial.println();

}

void SetRGB(int r, int g, int b) {
  analogWrite(RGBPin[0], r);
  analogWrite(RGBPin[1], g);
  analogWrite(RGBPin[2], b);
}

void BH1750_Init(int address){  
  Wire.beginTransmission(address);
  Wire.write(0x10); // 1 [lux] aufloesung
  Wire.endTransmission();
}

byte BH1750_Read(int address){
  
  byte i_BH1750=0;
  Wire.beginTransmission(address);
  Wire.requestFrom(address, 2);
  while(Wire.available()){
    buff_BH1750[i_BH1750] = Wire.read(); 
    i_BH1750++;
  }
  Wire.endTransmission();  
  return i_BH1750;
}

long bytesToInteger(byte b[4]) 
{
   long val = 0;
   val = ((long )b[0]) << 24;
   val |= ((long )b[1]) << 16;
   val |= ((long )b[2]) << 8;
   val |= b[3];
   return val;
}

void integerToBytes(long val, byte b[4]) 
{
   b[0] = (byte )((val >> 24) & 0xff);
   b[1] = (byte )((val >> 16) & 0xff);
   b[2] = (byte )((val >> 8) & 0xff);
   b[3] = (byte )(val & 0xff);
}


//////////////////////////////////// ReRec  ////////////////////////////////////

void ReRec()
{
  Serial.println("Wire.read ..............................................");

  noInterrupts();
  Sum_Reset = 0;   
  for(int j = 1 ; j <=5 ; j++)
  {
    act_type = j;
    act_state = recev[j] ;
    Sum_Reset = Sum_Reset + act_state;
    
    Serial.println(act_state);

    if(act_type == 1)
    {
      if(act_state == 1)
      {
        Serial.println("Led is ON");
        Led = HIGH;
      }
      else if(act_state == 0)
      {
        Serial.println("Led is OFF");
        Led = LOW;
      }
    }
    else if(act_type == 2)
    {
      if(act_state == 1)
      {
        Serial.println("Watering is ON");
        Watering = LOW;
      }
      else if(act_state == 0)
      {
        Serial.println("Watering is OFF");
        Watering = HIGH;
      }
    }
    else if(act_type == 3)
    {
      if(act_state == 1)
      {
        Serial.println("Vent_Fan is ON");
        Vent_Fan = LOW;
      }
      else if(act_state == 0)
      {
        Serial.println("Vent_Fan is OFF");
        Vent_Fan = HIGH;
      }
    }
    else if(act_type == 4)
    {
      if(act_state == 1)
      {
        Serial.println("Evap_cool is ON");
        Evap_cool = LOW;
      }
      else if(act_state == 0)
      {
        Serial.println("Evap_cool is OFF");
        Evap_cool = HIGH;
      }
    }
    else if(act_type == 5)
    {
      if(act_state == 1)
      {
        Serial.println("Fogger is ON");
        Fogger_ki = LOW;
      }
      else if(act_state == 0)
      {
        Serial.println("Fogger is OFF");
        Fogger_ki = HIGH;
      }
    } 
  }  
  
  if(Sum_Reset == 45)
  {
    Serial.println(" -------------------------------- Reset 99999");
    delay(2000);
    resetFunc();
    delay(10000);
  }

  // OUTPUT : Actuators :
  digitalWrite(Led_0_pin, Led);
  digitalWrite(Watering_1_pin, Watering);
  digitalWrite(Vent_Fan_2_pin, Vent_Fan);
  digitalWrite(Evap_cool_3_pin, Evap_cool);
  digitalWrite(Fogger_4_pin, Fogger_ki);    
  delay(1000);      
  checkRec = 0;
  interrupts(); 
}




//////////////////////////////////// setup  ////////////////////////////////////


void setup()
{ 
  //#if SERIAL
  Serial.begin(115200);
  Serial.println("Start [O] ............................................####### Start #######");
  //#endif
  
  pinMode(RGBPin[0], OUTPUT);
  pinMode(RGBPin[1], OUTPUT);
  pinMode(RGBPin[2], OUTPUT);
  
  pinMode(Led_0_pin, OUTPUT);
  pinMode(Watering_1_pin, OUTPUT);
  pinMode(Vent_Fan_2_pin, OUTPUT);
  pinMode(Evap_cool_3_pin, OUTPUT);
  pinMode(Fogger_4_pin, OUTPUT);

  Serial.println("Start [O] ...1");
  // Wire.begin(BH1750_address);
  // BH1750_Init(BH1750_address);  
  dht.setup(2); // data pin 2
  SetRGB(255, 255, 255);  // LED_RGB
   
  tBuf[0] = 173 ; // start1

  tBuf[1] = 0 ;  // Light_Int_val
  tBuf[2] = 0 ;  // Soil_Hum_val  
  tBuf[3] = 0 ;  // Air_RH_val
  tBuf[4] = 0 ;  // Air_Temp_val

  tBuf[5] = 0 ;  // Led
  tBuf[6] = 0 ;  // Watering
  tBuf[7] = 0 ;  // Vent_Fan
  tBuf[8] = 0 ;  // Evap_cool
  tBuf[9] = 0 ;   // Fogger

  tBuf[10] = 0 ;  // Light_Int_val _byte
  tBuf[11] = 0 ;  // Soil_Hum_val _byte
  tBuf[12] = 0 ;  // Air_RH_val _byte
  tBuf[13] = 0 ;  // Air_Temp_val _byte

  tBuf[14] = 237 ;// byte_stop 

  
  noInterrupts();
  Led = LOW;
  Watering = HIGH;
  Vent_Fan = HIGH;
  Evap_cool = HIGH;
  Fogger_ki = HIGH;
  
  digitalWrite(Led_0_pin, Led);
  digitalWrite(Watering_1_pin, Watering);
  digitalWrite(Vent_Fan_2_pin, Vent_Fan);
  digitalWrite(Evap_cool_3_pin, Evap_cool);
  digitalWrite(Fogger_4_pin, Fogger_ki);
  delay(1000);
  
  Wire.begin( I2C_ADDR );
  Wire.onRequest( i2c_response ); 
  Wire.onReceive( i2c_receive );  
  delay(1000);

  Serial.flush();
  while (Serial.available()) { char ch = Serial.read(); } 
  delay(1000);
  
  Serial.println("Start [O] ...2");
  //wdt_enable(WDTO_8S);
  interrupts();
  
  
}


////////////////////////////////////  loop  ////////////////////////////////////

void loop()
{    
  delay(50);
  now = millis();  
  if (now - lastSend > 1000) 
  {             
    lastSend = now;  
    GetStateValue();

    if (checkRec == 1)
    {
      ReRec();  //after i2c receive
      checkRec = 0;
    }    
  } 
}


////////////////////////////////////  i2c_receive  ////////////////////////////////////
void i2c_receive(int howMany) {
  while (0 < Wire.available()) {
    
    byte requestPos = Wire.read();
    for(int i=0;i < 7;i++)
    {
      recev[i] = Wire.read();
    }
  
    if(recev[0]==173 && recev[6]==237)
    {
      checkRec = 1;
    }
  }
}
//void i2c_receive(int howMany) 
//{
//  
//  Wire.read();
//  for(int i=0;i < 7;i++)
//  {
//    recev[i] = Wire.read();
//  }
//
//  if(recev[0]==173 && recev[6]==237)
//  {
//     //Serial.println("Wire.read ..............................................");
//
//    Sum_Reset = 0;   
//    for(int j = 1 ; j <=5 ; j++)
//    {
//      act_type = j;
//      act_state = recev[j] ;
//      Sum_Reset = Sum_Reset + act_state;
//      
//      //Serial.println(act_state);
//
//      if(act_type == 1)
//      {
//        if(act_state == 1)
//        {
//          // Serial.println("Led is ON");
//          Led = HIGH;
//        }
//        else if(act_state == 0)
//        {
//          // Serial.println("Led is OFF");
//          Led = LOW;
//        }
//      }
//      else if(act_type == 2)
//      {
//        if(act_state == 1)
//        {
//          // Serial.println("Watering is ON");
//          Watering = LOW;
//        }
//        else if(act_state == 0)
//        {
//          // Serial.println("Watering is OFF");
//          Watering = HIGH;
//        }
//      }
//      else if(act_type == 3)
//      {
//        if(act_state == 1)
//        {
//          // Serial.println("Vent_Fan is ON");
//          Vent_Fan = LOW;
//        }
//        else if(act_state == 0)
//        {
//          // Serial.println("Vent_Fan is OFF");
//          Vent_Fan = HIGH;
//        }
//      }
//      else if(act_type == 4)
//      {
//        if(act_state == 1)
//        {
//          // Serial.println("Evap_cool is ON");
//          Evap_cool = LOW;
//        }
//        else if(act_state == 0)
//        {
//          // Serial.println("Evap_cool is OFF");
//          Evap_cool = HIGH;
//        }
//      }
//      else if(act_type == 5)
//      {
//        if(act_state == 1)
//        {
//          // Serial.println("Fogger is ON");
//          Fogger_ki = LOW;
//        }
//        else if(act_state == 0)
//        {
//          // Serial.println("Fogger is OFF");
//          Fogger_ki = HIGH;
//        }
//      } 
//    }
//  }
//}
//
//

































