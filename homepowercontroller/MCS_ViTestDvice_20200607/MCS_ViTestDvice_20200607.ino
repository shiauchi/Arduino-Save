#include <LWiFi.h>
#include <MCS.h>
#include "DHT.h"
#include <PZEM004T.h>
PZEM004T pzem(2, 3); // (RX,TX) connect to TX,RX of PZEM   0708
float cur1;     //20200510電流的整數部份
float cur2;     //20200510電流的小數部份
float cur;      //0711 current
float vol1;     //20200510電壓的整數部份
float vol2;     //20200510電壓的小數部份
float vol;      //0711 voltage
float powe;     //0711 power
float kwh;      //0711 kWh
float temp=0;   //0412暫存功率計算的變數
float temp1;    //暫存總消耗功率的值，以防有0.01出現
float tp=0;     //總消耗功率
int   x=0;      //0412計算總消耗功率的迴圈變數
int   channelNo;        //0606顯示第幾電力支路變數
int   cc=600;   //0606契約容量(contract capacity)，超過契約容量系統開始做電力管制
int   pc=0;    //20200607電力支路1的電力控制(power controll)標籤,pc>0代表現在系統正在管控電力
char _lwifi_ssid[] = "chichi";
char _lwifi_pass[] = "0900540779";

MCSDevice mcs("DbreJyFf", "70czsc6qoroyAJju");
MCSControllerOnOff switch1("switch1");    //20200510彈性用電支路1網路開關
MCSControllerOnOff switch2("switch2");    //20200606彈性用電支路2網路開關
MCSControllerOnOff switch3("switch3");    //20200606彈性用電支路3網路開關
MCSControllerOnOff pwr_monitor("pwr_monitor");    //20200707是否自動省電的網路開關
MCSDisplayString pwr_channel("pwr_channel");  //20200606顯示第幾電力支路字串
MCSDisplayFloat v("v");     //電壓
MCSDisplayFloat i("i");     //電流
MCSDisplayFloat p("p");     //功率
MCSDisplayFloat e("e");     //總消耗功率
//==============================================================================

void setup()
{
  pinMode(2, OUTPUT);  //0506 IoT插座控制腳位
  Serial.begin(9600);
  Serial1.begin(9600);//p6/p7
  mcs.addChannel(v);
  mcs.addChannel(i);
  mcs.addChannel(p);
  mcs.addChannel(e);
  mcs.addChannel(switch1);    //20200510
  mcs.addChannel(switch2);    //20200606
  mcs.addChannel(switch3);    //20200606
  mcs.addChannel(pwr_channel);  //20200606
  mcs.addChannel(pwr_monitor);  //20200707
  Serial.println("Wi-Fi connecting......");
  while (WiFi.begin(_lwifi_ssid, _lwifi_pass) != WL_CONNECTED) {
    delay(1000);
  }
  Serial.println("Wi-Fi connected successfully!");
  while (!mcs.connected()) {
    mcs.connect();
  }
  Serial.println("MCS connected successfully!");
  Serial.begin(9600);
}
//============================================================================

void loop()
{
//  digitalWrite(2, HIGH);   //0506  TEST

  while (!mcs.connected()) {
    mcs.connect();
    if (mcs.connected()) {
      Serial.println("MCS 已重新連線");
    }
  }
  mcs.process(100);

  cur1=Serial1.read();      //20200510
  cur2=Serial1.read();      //20200510   
  vol1=Serial1.read();      //20205010
  vol2=Serial1.read();
  channelNo=Serial1.read();          //20200606
//<<20200422
    cur=cur1+(cur2/100);  //20200510
    vol=vol1+(vol2/100);  //20205010
    powe=(cur*vol);     
    temp=powe;
    tp=temp+tp;
    x=x+1;
//-----------------------------------------------------------------------
    if(x==3) 
    {
      kwh=tp;
      x=0;
      temp=0;
      tp=0;
    }
 //-----------------------------------------------------------------------   
//20200412>>
  if(vol>=0) 
  {
    v.set(vol);
    Serial.print("voltage : ");
    Serial.println(vol);
  }
//------------------------------------------------------------------------  
  if(cur>=0)
  {
    i.set(cur);   
    Serial.print("current :");
    Serial.println(cur);
  }
 //----------------------------------------------------------------------- 
  if(powe>=0)
  {
    p.set(powe);
    Serial.print("power :");
    Serial.println(powe);
  }
//-------------------------------------------------------------------------  
  if(kwh>=0)
  {
    if(kwh==0.01)
    {
     e.set(temp1);  
//     mcsswitch();       
     Serial.print("total power :");
     Serial.println(temp1);
    Serial.println();
    }
  else
    {
     e.set(kwh);
     disp_channel();
     temp1=kwh;
     mcsswitch();
     Serial.print("total power :");
     Serial.println(kwh);
     Serial.println();
    }
  }
//------------------------超過契約容量(cc)系統開始做電力管制------------------------------------------------
    if(pwr_monitor.value())   //自動節電開關
    {
          if (temp1 > cc)    
          {
              if(pc<3)
              {
                  digitalWrite(2, HIGH);   //電力支路1(p2)斷電
                  digitalWrite(3, LOW);    //電力支路2(p3)繼續供電
                  digitalWrite(4, LOW);    //電力支路3(p4)繼續供電
              } 
//-----------------------------------------------------------------------------------
              if(pc>=3 and pc<5)
              {
                  digitalWrite(2, HIGH);   //電力支路1(p2)斷電
                  digitalWrite(3, HIGH);   //電力支路2(p3)斷電
                  digitalWrite(4, LOW);    //電力支路3(p4)繼續供電           
              } 
//-----------------------------------------------------------------------------------
              if(pc==5)
              {
                  digitalWrite(2, HIGH);   //電力支路1(p2)斷電
                  digitalWrite(3, HIGH);   //電力支路2(p3)斷電
                  digitalWrite(4, HIGH);   //電力支路3(p4)斷電
              } 
              pc=pc+1;
              if(pc>5)
              pc=5;
//-----------------------------------------------------------------------------------
          }  
          else
          {
            digitalWrite(2, LOW);    //電力支路1(p2)繼續供電
            digitalWrite(3, LOW);    //電力支路2(p3)繼續供電
            digitalWrite(4, LOW);    //電力支路3(p4)繼續供電
            pc=0;
          } 
    }
//-----------------------------------------------------------------------------------
if(pwr_monitor.updated())
  {
        if(pwr_monitor.value())   //自動節電開關
        {
              if (temp1 > cc)    
              {
                  if(pc<3)
                  {
                      digitalWrite(2, HIGH);   //電力支路1(p2)斷電
                      digitalWrite(3, LOW);    //電力支路2(p3)繼續供電
                      digitalWrite(4, LOW);    //電力支路3(p4)繼續供電
                  } 
    //-----------------------------------------------------------------------------------
                  if(pc>=3 and pc<5)
                  {
                      digitalWrite(2, HIGH);   //電力支路1(p2)斷電
                      digitalWrite(3, HIGH);   //電力支路2(p3)斷電
                      digitalWrite(4, LOW);    //電力支路3(p4)繼續供電           
                  } 
    //-----------------------------------------------------------------------------------
                  if(pc>=5)
                  {
                      digitalWrite(2, HIGH);   //電力支路1(p2)斷電
                      digitalWrite(3, HIGH);   //電力支路2(p3)斷電
                      digitalWrite(4, HIGH);   //電力支路3(p4)斷電
                  } 
                  pc=pc+1;
                  if(pc>5)
                  pc=5;
    //-----------------------------------------------------------------------------------
              }  
              else
              {
                digitalWrite(2, LOW);    //電力支路1(p2)繼續供電
                digitalWrite(3, LOW);    //電力支路2(p3)繼續供電
                digitalWrite(4, LOW);    //電力支路3(p4)繼續供電
                pc=0;
              } 
        }    
  }

//----------------------------------------------------------------------------------
  delay(8000);
}
//==========================================================================


void mcsswitch()      //20200510 mcs的開關控制
{

//---------------------------0506 IoT插座啟閉控制程式------------------------------------------------------------
        if (switch1.value())         
        {
            digitalWrite(2, LOW);    //電力支路1(p2)繼續供電
        }  
        else
        {
             digitalWrite(2, HIGH);   //電力支路1(p2)斷電
        }
//----------------------------0506 IoT插座啟閉控制程式--------------------------------------------
        if (switch2.value())         
        {
            digitalWrite(3, LOW);    //電力支路2(p3)繼續供電
        }  
        else
        {
             digitalWrite(3, HIGH);   //電力支路2(p3)斷電
        }
//----------------------------0506 IoT插座啟閉控制程式--------------------------------------------
         if (switch3.value())         
        {
            digitalWrite(4, LOW);    //電力支路3(p4)繼續供電
        }  
        else
        {
             digitalWrite(4, HIGH);   //電力支路3(p4)斷電
        }
//------------------------------------------------------------------------
          
  if(switch1.updated())
  {
        if (switch1.value())         
        {
            digitalWrite(2, LOW);    //電力支路1(p2)繼續供電
        }  
        else
        {
            digitalWrite(2, HIGH);   //電力支路1(p2)斷電
        }
  }
//-----------------------------------------------------------------------

  if(switch2.updated())
  {
        if (switch2.value())         
        {
            digitalWrite(3, LOW);    //電力支路2(p3)繼續供電
        }  
        else
        {
             digitalWrite(3, HIGH);   //電力支路2(p3)斷電
        }
  }
//------------------------------------------------------------------------
 
  if(switch3.updated())
  {
      if (switch3.value())
      {
        digitalWrite(4, LOW);    //電力支路3(p4)繼續供電
      }
      else
      {
        digitalWrite(4, HIGH);    //電力支路3(p4)斷電
      }  
  }
//-------------------------------------------------------------------------
/*  if(pwr_monitor.updated())
  {
        if(pwr_monitor.value())   //自動節電開關
        {
                if (temp1 > cc)    //超過契約容量(cc)系統開始做電力管制
                {
                    if(pc==0)
                    {
                        digitalWrite(2, HIGH);   //電力支路1(p2)斷電
                        digitalWrite(3, LOW);    //電力支路2(p3)繼續供電
                        digitalWrite(4, LOW);    //電力支路3(p4)繼續供電
                        pc=pc+1;                 //pc=1              
                    } 
      //-----------------------------------------------------------------------------------
                    if(pc==1)
                    {
                        digitalWrite(2, HIGH);   //電力支路1(p2)斷電
                        digitalWrite(3, HIGH);   //電力支路2(p3)斷電
                        digitalWrite(4, LOW);    //電力支路3(p4)繼續供電
                        pc=pc+1;                 //pc=2              
                    } 
      //-----------------------------------------------------------------------------------
                    if(pc==2)
                    {
                        digitalWrite(2, HIGH);   //電力支路1(p2)斷電
                        digitalWrite(3, HIGH);   //電力支路2(p3)斷電
                        digitalWrite(4, LOW);    //電力支路3(p4)斷電
                                                 //pc=2              
                    } 
      //-----------------------------------------------------------------------------------
                }  
                else
                {
                  digitalWrite(2, LOW);    //電力支路1(p2)繼續供電
                  digitalWrite(3, LOW);    //電力支路2(p3)繼續供電
                  digitalWrite(4, LOW);    //電力支路3(p4)繼續供電
                } 
        }
  }*/
//-------------------------------------------------------------------------------
} 
//===============================================================================


void disp_channel()      //20200606 計算並顯示第幾個電力支路
{
    if(channelNo==1) 
      pwr_channel.set("電力支路1");    //20200606
    if(channelNo==2) 
      pwr_channel.set("電力支路2");    //20200606
    if(channelNo==3) 
      pwr_channel.set("電力支路3");    //20200606
}
//=========================================================================
