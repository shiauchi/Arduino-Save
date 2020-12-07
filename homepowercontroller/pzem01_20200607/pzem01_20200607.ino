#include <Wire.h>
#include <SeeedOLED.h>
#include <PZEM004T.h>
#include <SoftwareSerial.h>    //0711

SoftwareSerial ltk7697(4, 5); //0711 RX, TX
PZEM004T pzem(2, 3); // (RX,TX) connect to TX,RX of PZEM

unsigned int V;
byte I;
int channelNo=1;      //0606計算顯示第幾電力支路變數
IPAddress ip(192, 168, 1, 1);

void setup()
{
  Wire.begin();
  pinMode(MISO, OUTPUT);
  pinMode(6, OUTPUT);  //1205 relay control pin
  pinMode(7, OUTPUT);  //1205 relay control pin
  pinMode(8, OUTPUT);  //1205 relay control pin
//  pinMode(9, OUTPUT);  //超過500W就關電
  Serial.begin(115200);  // 用於手動輸入文字
  ltk7697.begin(9600);
  pzem.setAddress(ip);
  SPCR |= _BV(SPE);
  // turn on interrupts
  SPCR |= _BV(SPIE);
}

unsigned int y;
// SPI interrupt routine
ISR (SPI_STC_vect)
{
  byte c = SPDR;
  switch (c)
  {
    case 0:
      SPDR = V;
      break;
    case 1:
      SPDR = I;
      break;
  }
}

//unsigned int i=0;
long t;
long preT;
long delyT;
float e = 0; //總消耗功率
float i = 0;
float v = 0;
float p = 0;
float p1 = 0;
float p2 = 0;

void loop()
{
  digitalWrite(6, HIGH); //1205
  getdata();
  p1 = p;
  digitalWrite(6, LOW); //1205

  digitalWrite(7, HIGH); //1205
  getdata();
  p2 = p;
  digitalWrite(7, LOW); //1205
  
  digitalWrite(8, HIGH); //1205
  getdata();
  e = p1 + p2 + p; //1205 總消耗功率
  digitalWrite(8, LOW); //1205
}

void getdata()
{
  int temp;       //20200510
  int int_i;      //20200510:儲存電流的整數值
  int float_i;    //20200510:儲存電流的小數值
  int int_v;      //20200510:儲存電壓的整數值
  int float_v;    //20200510:儲存電壓的小數值
  
  i = pzem.current(ip);
  //<<0609test
  Serial.print("Pzem Current(A): ");
  Serial.println(i);
  //0609test>>
  temp=i*100;    //20200510
  int_i=temp/100;   //20200510
  float_i=temp%100; //20200510 
       
  v = pzem.voltage(ip);
  temp=v*100;       //20200510
  int_v=temp/100;   //20200510
  float_v=temp%100;  //20200510
//<<20200422
  if (v < 0) v = 0;
  if (i < 0) i = 0;
  if (p < 0) p = 0;
  i=i*10;      //電流以0.1A的單位顯示
  p=(i*v)/10;
//20200422>>

  channelNo=channelNo+1;
  if(channelNo==4)
    channelNo=1;
  
  ltk7697.write(int_i);   //20200510
  ltk7697.write(float_i); //20200510
  ltk7697.write(int_v);   //20200510
  ltk7697.write(float_v); //20200510
  ltk7697.write(channelNo);       //20200606
  delay(8000);  //1223 test
  //0711>
  
//  <<1223測試
    Serial.print("Time: ");
    Serial.println(millis()); //prints time since program started
    Serial.print("Current(0.1A): ");
    Serial.println(i);
    Serial.print("Voltage: ");
    Serial.println(v);
    Serial.print("Power: ");
    Serial.println(p);
    Serial.print("Total Power: ");
    Serial.println(e);
    Serial.print("Power Channel NO: ");
    Serial.println(channelNo);
    Serial.println();
   //1223測試>> 
}








