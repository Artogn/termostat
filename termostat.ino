// 
//   FILE:  dht11_test1.pde
// PURPOSE: DHT11 library test sketch for Arduino
//
#include <SevenSegmentDisplay.h>
#include <dht11.h>
#include <EEPROM.h>

dht11 DHT;
byte cUp, cDown, cStab, cPart, dPart, Tstab;
byte T, Told, cntUp, cntDown, cntStab, cntHeat, Step, Delta;
boolean flUp, flDown, flStab;
int addrEEPROM, dataEEPROM;
int chk;
unsigned long cntT;

#define DHT11_PIN 11
#define Relay 12

SevenSegmentDisplay<true, BiDigit<9, 10> > ss(2, 3, 4, 5, 6, 7, 8);

void soft_Reset()
{
  asm volatile ("  jmp 0");  
} 

void setup(){
  pinMode(Relay, OUTPUT);
  digitalWrite(Relay, HIGH);
  Step = 1;
  cntT = 0;
  Serial.begin(9600);
  Serial.println("DHT TEST PROGRAM ");
  Serial.print("LIBRARY VERSION: ");
  Serial.println(DHT11LIB_VERSION);
  Serial.println();
  Serial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C)");
  dataEEPROM = EEPROM.read(0);
  if (dataEEPROM != 55){
     EEPROM.write(1, 3);//cUp
     EEPROM.write(2, 3);//cDown
     EEPROM.write(3, 50);//cStab
     EEPROM.write(4, 10);//cPart
     EEPROM.write(5, 10);//dPart
     EEPROM.write(6, 24);//Tstab
     EEPROM.write(0, 55);//Byte's first write
     }
  cUp   = EEPROM.read(1);
  cDown = EEPROM.read(2);
  cStab = EEPROM.read(3);
  cPart = EEPROM.read(4);
  dPart = EEPROM.read(5);
  Tstab = EEPROM.read(6);
}

void loop(){
  T = 88;
  Serial.print("DHT11, \t");
  chk = DHT.read(DHT11_PIN);    // READ DATA
  switch (chk){
    case DHTLIB_OK:  
                T = DHT.temperature - 1;
                Serial.print("OK,\t");
                Serial.print(cntT,1);
                Serial.print(",\t");
                Serial.println(T,1);
                break;
    case DHTLIB_ERROR_CHECKSUM: 
                Serial.print("Checksum error,\t"); 
                break;
    case DHTLIB_ERROR_TIMEOUT: 
                Serial.print("Time out error,\t"); 
                break;
    default: 
                Serial.print("Unknown error,\t");
                break;
  }
 // DISPLAT DATA

  regulir();
  procEEPROM();
  ss.print(T, -1, 10000);
  cntT++;
}

void regulir(){
  switch (Step){
    case 1:
          Told = T;
          flUp = flDown = flStab = false;
          cntUp = cntDown = cntStab = 0;
          Step = 2;
          break;
    case 2:
          if(T == Told){cntStab++;}
          if(T >  Told){Told = T; cntUp++; cntStab = 0;}
          if(T <  Told){Told = T; cntDown++; cntStab = 0;}
          if(cntUp   == cUp)  {flUp = true;   Step = 3;}
          if(cntDown == cDown){flDown = true; Step = 3;}
          if(cntStab == cStab){flStab = true; Step = 3;}
          break;
    case 3: 
          Delta = abs(T - Tstab);
          if((Delta < 2)&&flStab){Step = 1; break;}
          if((T > Tstab) || (T == Tstab))
             if (flUp || flStab){Step = 4; /* ON COOLER */ break;}
          if((T < Tstab) || (T == Tstab))
             if (flDown || flStab){Step = 5; cntHeat = cPart + Delta*dPart; digitalWrite(Relay, LOW); break;}//ON HEATER
          Step = 1;//any condition wasn't satisfied   
          break;
    case 4: break;
    case 5: 
          cntHeat--;
          if((T > Tstab-1) || (0 == cntHeat)){digitalWrite(Relay, HIGH); Step = 1;}//OFF HEATER
          break;
  }//end switch
}

void procEEPROM(){
  if (Serial.available() > 0) {
         // read the incoming strings like "5,220": 5 - addr 220 - data
         addrEEPROM = Serial.parseInt();
         if(addrEEPROM == 666) soft_Reset();//RESET 
         // do it again:
         dataEEPROM = Serial.parseInt();
         // say what you got:
         Serial.print("I received: ");
         Serial.print("addr: ");
         Serial.print(addrEEPROM, DEC);
         Serial.print("    data: ");
         Serial.println(dataEEPROM, DEC);
         EEPROM.write(addrEEPROM, dataEEPROM);
  }  
}

