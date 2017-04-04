#include <SoftwareSerial.h>
#include <Wire.h>
#include <DHT.h>
#include <avr/wdt.h>

SoftwareSerial mySerial(12, 13);

//init pin
#define DHTTYPE21 DHT21
#define DHTTYPE11 DHT11


const int actModePin = 7;
int chkActModeAgo ;

const int SoilPin1 = A1;
const int SoilPin2 = A2;
const int waterPin = A0;
const int dhtPin1 = 8;
const int dhtPin2 = 9;
const int dhtPin3 = 10;
const int BH1750_address = 0x23;
byte buff_BH1750[2];

DHT dht1(dhtPin1, DHTTYPE21);
DHT dht2(dhtPin2, DHTTYPE21);
DHT dht3(dhtPin3, DHTTYPE11);

const int relayPin[6] = { 3, 4, 5, 6 , 2 , 11};

//init sensor_val
const int sensor_Count = 10;
int sensor_val[sensor_Count];

//init act_state
const int act_Count = 6;
uint8_t act_state[act_Count];
uint8_t act_read[act_Count];

//init Serial
byte tBuf[28];
byte rBuf[8];

//Var time delay
unsigned long previousMillis;
const long interval = 1000;


#define resetFunc()         \
  do                        \
  {                         \
    wdt_enable(WDTO_15MS);  \
    for(;;)                 \
    {                       \
    }                       \
  } while(0)


void BH1750_Init(int address) {
  Wire.beginTransmission(address);
  Wire.write(0x10); // 1 [lux] aufloesung
  Wire.endTransmission();
}

byte BH1750_Read(int address) {
  byte i_BH1750 = 0;
  Wire.beginTransmission(address);
  Wire.requestFrom(address, 2);
  while (Wire.available()) {
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


//////////////////////////////////// getValue  ////////////////////////////////////

void GetValue() {

}


//////////////////////////////////// setup  ////////////////////////////////////

void setup() {
  // Serial
  Serial.begin(115200);
  Serial.print("HO HO HO HOOO.-------------------------------------");
  delay(100);

  mySerial.begin(112500);
  delay(100);

  // Sensor and Act
  pinMode(actModePin, INPUT);
  for (int i = 0; i < act_Count; i++)
  {
    act_state[i] = HIGH;
    pinMode(relayPin[i], OUTPUT);
    digitalWrite(relayPin[i], act_state[i]);
    //    Serial.print(digitalRead(relayPin[i]));
  }

  Wire.begin();
  BH1750_Init(BH1750_address);

  dht1.begin();
  dht2.begin();
  dht3.begin();

  // Sent
  tBuf[0] = 173 ; // start1

  tBuf[1] = 0 ; // Led
  tBuf[2] = 0 ; // Watering
  tBuf[3] = 0 ; // Fogger
  tBuf[4] = 0 ; // Evap_cool
  tBuf[5] = 0 ; // Vent_Fan
  tBuf[6] = 0 ; // Vent_Fan

  tBuf[7] = 0 ;  // Light_Int_val
  tBuf[8] = 0 ;  // Soil_Hum_val 1
  tBuf[9] = 0 ;  // Soil_Hum_val 2
  tBuf[10] = 0 ;  // Air_RH_val   1
  tBuf[11] = 0 ; // Air_Temp_val 1
  tBuf[12] = 0 ; // Air_RH_val   2
  tBuf[13] = 0 ; // Air_Temp_val 2
  tBuf[14] = 0 ; // Air_RH_val   3
  tBuf[15] = 0 ; // Air_Temp_val 3
  tBuf[16] = 0 ; // water_val

  tBuf[17] = 0 ;  // Light_Int_val
  tBuf[18] = 0 ;  // Soil_Hum_val 1
  tBuf[19] = 0 ;  // Soil_Hum_val 2
  tBuf[20] = 0 ;  // Air_RH_val   1
  tBuf[21] = 0 ; // Air_Temp_val 1
  tBuf[22] = 0 ; // Air_RH_val   2
  tBuf[23] = 0 ; // Air_Temp_val 2
  tBuf[24] = 0 ; // Air_RH_val   3
  tBuf[25] = 0 ; // Air_Temp_val 3
  tBuf[26] = 0 ; // water_val

  tBuf[27] = 237 ; // byte_stop


  //Set watch Dog timmer
  wdt_enable(WDTO_8S); //wdt_reset();
  previousMillis = 0;
}




////////////////////////////////////  loop  ////////////////////////////////////
void loop() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    wdt_reset();
    Serial.println();
    Serial.println(".");

    // Read act
    Serial.print("act :");
    for (int i = 0; i < act_Count; i++)
    {
      act_read[i] = digitalRead(relayPin[i]);
      Serial.print(digitalRead(relayPin[i]));
    }
    Serial.println();

    /////////////////  Sensor //////////////////
    for (int i = 0; i < sensor_Count; i++)
    {
      sensor_val[i] = 0;
    }

    if (BH1750_Read(BH1750_address) == 2) {
      float valf = 0;
      valf = ((buff_BH1750[0] << 8) | buff_BH1750[1]) / 1.2;
      if (valf < 0)
      {
        sensor_val[0] = 65535;
      }
      else
      {
        sensor_val[0] = valf + 50;
      }
    }

    Serial.print("Light_Int_val \t: ");
    Serial.print(sensor_val[0], DEC);
    Serial.println(" lx");

    //soil
    //1
    sensor_val[1] = analogRead(SoilPin1);
    Serial.print("Soil_Air_RH_val 1: " );
    Serial.println(sensor_val[1]);

    //2
    sensor_val[2] = analogRead(SoilPin2);
    Serial.print("Soil_Air_RH_val 2: " );
    Serial.println(sensor_val[2]);

    //DHT
    float humidity1 = dht1.readHumidity();
    float temperature1 = dht1.readTemperature();
    sensor_val[3] = humidity1 * 10;
    sensor_val[4] = temperature1 * 10;

    float humidity2 = dht2.readHumidity();
    float temperature2 = dht2.readTemperature();
    sensor_val[5] = humidity2 * 10;
    sensor_val[6] = temperature2 * 10;

    float humidity3 = dht3.readHumidity();
    float temperature3 = dht3.readTemperature();
    sensor_val[7] = humidity3 * 10;
    sensor_val[8] = temperature3 * 10;


    // 1
    Serial.print("Air_RH_val 1 \t: ");
    Serial.print(sensor_val[3], 1);
    Serial.println(" %RH");

    Serial.print("Air_Temp_val 1 \t: ");
    Serial.print(sensor_val[4], 1);
    Serial.println(" C");

    // 2
    Serial.print("Air_RH_val 2 \t: ");
    Serial.print(sensor_val[5], 1);
    Serial.println(" %RH");

    Serial.print("Air_Temp_val 2 \t: ");
    Serial.print(sensor_val[6], 1);
    Serial.println(" C");

    // 3
    Serial.print("Air_RH_val 3 \t: ");
    Serial.print(sensor_val[7], 1);
    Serial.println(" %RH");

    Serial.print("Air_Temp_val 3 \t: ");
    Serial.print(sensor_val[8], 1);
    Serial.println(" C");

    //water
    sensor_val[9] = analogRead(waterPin);
    Serial.print("water_val : " );
    Serial.println(sensor_val[9]);

    ///////////// !end Sensor /////////////


    ///////////// act mode /////////////
    //act mode
    int chkActMode = digitalRead(actModePin);
    Serial.print("Mode act :");
    Serial.println(chkActMode);
    if (( chkActModeAgo + chkActMode ) == 1)
    {
      Serial.println("Mode change !");
    }

    if (digitalRead(actModePin) == 0) // Mode Switch manual
    {
      // write act
      for (int i = 0; i < act_Count; i++)
      {
        digitalWrite(relayPin[i], HIGH);
      }
    }
    else
    {
      for (int i = 0; i < act_Count; i++)
      {
        digitalWrite(relayPin[i], act_state[i]);
      }
    }

    chkActModeAgo = chkActMode;

    ///////////// !end act /////////////



    ///////////// Set tBuf /////////////
    //set act state
    for (int i = 0; i < act_Count; i++)
    {
      tBuf[i + 1] = act_read[i];
    }

    //set val sensor
    for (int i = 0; i < sensor_Count; i++)
    {
      byte bVal[4];
      integerToBytes(sensor_val[i], bVal);
      for (int i = 0; i < 4; ++i) {
      }
      tBuf[i + act_Count + 1] = bVal[3];
      tBuf[i + act_Count + sensor_Count + 1] = (int)bVal[2];
    }

    //    //print tBuf
    //    Serial.println(tBuf[0]);
    //    Serial.println(tBuf[1]);
    //    Serial.println(tBuf[2]);
    //    Serial.println(tBuf[3]);
    //    Serial.println(tBuf[4]);
    //    Serial.println(tBuf[5]);
    //    Serial.println(tBuf[6]);
    //    Serial.println(tBuf[7]);
    //    Serial.println(tBuf[8]);
    //    Serial.println(tBuf[9]);
    //    Serial.println(tBuf[10]);
    //    Serial.println(tBuf[11]);
    //    Serial.println(tBuf[12]);
    //    Serial.println(tBuf[13]);
    //    Serial.println(tBuf[14]);
    //    Serial.println(tBuf[15]);
    //    Serial.println(tBuf[16]);
    //    Serial.println(tBuf[17]);
    //    Serial.println(tBuf[18]);
    //    Serial.println(tBuf[19]);
    //    Serial.println(tBuf[20]);
    //    Serial.println(tBuf[21]);
    //    Serial.println(tBuf[22]);
    //    Serial.println(tBuf[23]);
    //    Serial.println(tBuf[24]);
    //    Serial.println(tBuf[25]);
    //    Serial.println(tBuf[26]);
    //    Serial.println(tBuf[27]);

    ///////////// !Set tBuf /////////////



    ///////////// Sent tBuf to esp /////////////
    mySerial.write(tBuf, sizeof(tBuf));

    ///////////// !Sent tBuf to esp /////////////



    // -------------- code here --------------



    if (mySerial.overflow()) {
      Serial.println("SoftwareSerial overflow!");
    }

    if (mySerial.isListening())
    {
    }

    Serial.println("________________________");
    Serial.println();

  }
}




