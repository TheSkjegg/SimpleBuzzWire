// Description: 
//    Buzz Wire game with timer and high score list. Start/stop pads ensure that timing starts and stops, enabling a precise timing
//    of each game.
//
// Usage:
//    Developed on the Seed Studio XIAO SAMD21 board, but should be compatible with all "Arduino UNO" boards.
//    Simply upload this code to your board and test your Buzz Wire game!
//    NB: High score list will be reset when uploading this code. To keep existing high scores, you must fill them into the EEPROM init in setup below.
//
// Author: 
//    The Skjegg 22/02/2024
//
// Versions:
//    v001: First basic working version
//    v002: Added 1,2 or 3 place indication
//    v003: Added Hello and other texts

#include "LedControl.h"
#include "FlashAsEEPROM.h"

// IO def
#define StartPad1 3
#define StartPad2 4
#define WireIO 5 // Interrupt
#define Buzz 6
#define Led1 0
#define Led2 1
#define Led3 2

LedControl lc=LedControl(9,7,8,1);
// 9 --> DIN, 7 --> CLK, 8 --> CS, 1 --> 1pcs LED driver

// Globale vars
bool nono = false;

// Timing
unsigned long Start, Elapsed;
unsigned long MaxTime = 59000;

byte flashHighScores = 0;
boolean newStart = false;
volatile boolean failed = false;
volatile byte failed_no = 0;
byte listePlass = 0;

// EEPROM
long Score=0;

void setup() {


  if (!EEPROM.isValid()){
    EEPROMWriteLong(20, 0); // 1 place - to keep a high score, insert high score in milliseconds (last para)
    EEPROMWriteLong(40, 0); // 2 place - to keep a high score, insert high score in milliseconds (last para)
    EEPROMWriteLong(60, 0); // 3 place - to keep a high score, insert high score in milliseconds (last para)
  }

  //IO
  pinMode(StartPad1, INPUT);
  digitalWrite(StartPad1, HIGH); // pullup
  pinMode(StartPad2, INPUT);
  digitalWrite(StartPad2, HIGH); // pullup
  pinMode(WireIO, INPUT);
  digitalWrite(WireIO, HIGH); // pullup
  pinMode(Led1, OUTPUT);
  digitalWrite(Led1, LOW);
  pinMode(Led2, OUTPUT);
  digitalWrite(Led1, LOW);
  pinMode(Led3, OUTPUT);
  digitalWrite(Led1, LOW);

  // 7segment
  lc.shutdown(0,false);
  lc.setIntensity(0,3);
  for(signed int i=8;i>=-5;i--){ // Hello start message
    lc.clearDisplay(0);
    lc.setChar(0,i+4,'H',false);
    lc.setChar(0,i+3,'E',false);
    lc.setChar(0,i+2,'L',false);
    lc.setChar(0,i+1,'L',false);
    lc.setChar(0,i,'0',false);
    if(i==1){
      successSound();
      delay(1500);
    }
    else
      delay(50);
}
  lc.clearDisplay(0);
  delay(500);
  displayTime(0);

  attachInterrupt(WireIO, busted, CHANGE);
  delay(100);
  detachInterrupt(WireIO);
  failed_no = 0;
  failed = false;
}

void dispLoser(){
  lc.clearDisplay(0);
  lc.setChar(0,5,'L',false);
  lc.setChar(0,4,'0',false);
  lc.setChar(0,3,'5',false);
  lc.setChar(0,2,'E',false);
  lc.setChar(0,1,'A',false);
  }

void dispNoRec(){
  lc.clearDisplay(0);
  lc.setLed(0,6,1,true);
  lc.setLed(0,6,2,true);
  lc.setLed(0,6,3,true);
  lc.setLed(0,6,5,true);
  lc.setLed(0,6,6,true);
  lc.setChar(0,5,'0',false);
  lc.setChar(0,3,'A',false);
  lc.setChar(0,2,'E',false);
  lc.setLed(0,1,1,true);
  lc.setLed(0,1,4,true);
  lc.setLed(0,1,5,true);
  lc.setLed(0,1,6,true);
  delay(2000);
  lc.clearDisplay(0);
}

void dispPlace(int place){
  lc.clearDisplay(0);
  if(place == 1)
    lc.setChar(0,7,'1',true);
  else if(place == 2)
    lc.setChar(0,7,'2',true);
  else
    lc.setChar(0,7,'3',true);
  lc.setChar(0,5,'P',false);
  lc.setChar(0,4,'L',false);
  lc.setChar(0,3,'A',false);
  lc.setLed(0,2,1,true);
  lc.setLed(0,2,4,true);
  lc.setLed(0,2,5,true);
  lc.setLed(0,2,6,true);
  lc.setChar(0,1,'E',false);
  delay(2000);
  lc.clearDisplay(0);
}

void ReadHighscore(int StartAdr){
  Score = ReadScore(StartAdr);
}

void EEPROMWriteLong(int EepromAdr, long value){
      byte four = (value & 0xFF);
      byte three = ((value >> 8) & 0xFF);
      byte two = ((value >> 16) & 0xFF);
      byte one = ((value >> 24) & 0xFF);

      EEPROM.write(EepromAdr, four);
      EEPROM.write(EepromAdr + 1, three);
      EEPROM.write(EepromAdr + 2, two);
      EEPROM.write(EepromAdr + 3, one);

      EEPROM.commit();
}

long EEPROMReadLong(int EepromAdr){
      long four = EEPROM.read(EepromAdr);
      long three = EEPROM.read(EepromAdr + 1);
      long two = EEPROM.read(EepromAdr + 2);
      long one = EEPROM.read(EepromAdr + 3);

      return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

long ReadScore (int EepromAdr){
  return EEPROMReadLong(EepromAdr+10);
}

int CheckScore (long score){
  int p=1;
  int adr=20;
  long StoredScore;
  for(int i=20;i<61;i=i+20){
    StoredScore = EEPROMReadLong(i);
    if(StoredScore == 0)
      p=p;
    else if(score > StoredScore)
      p++;
  }
  return p;
}

void DeleteScore(int p){
  byte d;
  int adr = p*20 - 10;
  for(int h=adr;h<=30;h=h+20){
    for(int i=0;i<20;i++){
    d = EEPROM.read(h + 20 + i);
    EEPROM.write(h + i, d);
    }
  }
  EEPROMWriteLong(60, 0); // Delete last HS
}

bool InsertScore(long Score){
  byte d;
  int p =  CheckScore(Score);
  int adr = p*20 - 10;
  if(p == 3)
    EEPROMWriteLong((adr+10), Score);
  else if(p < 3){
    for(int h=50;h>=adr;h=h-20){
      for(int i=0;i<20;i++){
        d = EEPROM.read(h + i);
        EEPROM.write(h + 20 + i, d);
        }
    }
    EEPROMWriteLong((adr+10), Score);
  }
  return true;
}

void displayTime(unsigned long millisek){
  unsigned long over;
  int num;
  
  num=int(millisek/600000);  // m_tens
  //lc.setDigit(0,6,num,false);
  over=millisek%600000;
  num=int(over/60000);   // m_ones
  //lc.setDigit(0,5,num,true);
  over=over%60000;
  num=int(over/10000);   // s_tens
  lc.setDigit(0,4,num,false);
  over=over%10000;
  num=int(over/1000);    // s_ones
  lc.setDigit(0,3,num,true);
  over=over%1000;
  num=int(over/100);  // ms_hundreds
  lc.setDigit(0,2,num,false);
  over=over%100;
  num=int(over/10);           // ms_tens
  lc.setDigit(0,1,num,false);
  over=over%10;
  num=int(over);              // ms_ones
  lc.setDigit(0,0,num,false);
  
} // displayTime

void timeToChar(unsigned long millisek, char *Time){

  unsigned long over;
  int num;
  char buff[10];

  num=int(millisek/600000);   //Minutes, tens (m_tens)
  itoa(num,buff,10);
  Time[0] = buff[0];          //Minutes, ones (m_ones)
  over=millisek%600000;
  num=int(over/60000);
  itoa(num,buff,10);
  Time[1] = buff[0];
  Time[2] = ':';
  over=over%60000;            //Seconds, tens (s_tens)
  num=int(over/10000);
  itoa(num,buff,10);
  Time[3] = buff[0];
  over=over%10000;            //Second, ones (s_ones)
  num=int(over/1000);
  itoa(num,buff,10);
  Time[4] = buff[0];
  Time[5] = ':';
  over=over%1000;             //Millisec, hundreds (ms_hundreds)
  num=int(over/100);
  itoa(num,buff,10);
  Time[6] = buff[0];
  over=over%100;              //Millisec, tens (ms_tens)
  num=int(over/10);
  itoa(num,buff,10);
  Time[7] = buff[0];
  over=over%10;               //Millisec, ones (ms_ones)
  num=int(over);
  itoa(num,buff,10);
  Time[8] = buff[0];
}

void successSound(){
    pinMode(Buzz, OUTPUT);
    tone(Buzz, 523);
    delay(100);
    noTone(Buzz);
    delay(50);
    tone(Buzz, 1047);
    delay(500);
    noTone(Buzz);
    pinMode(Buzz, INPUT);
}

void warningSound(){
    pinMode(Buzz, OUTPUT);
    tone(Buzz, 196); // Note G3
    delay(100);
    noTone(Buzz);
    pinMode(Buzz, INPUT);
}

void failedSound(){
    pinMode(Buzz, OUTPUT);
    tone(Buzz, 98); // Note G2
    delay(500);
    noTone(Buzz);
    pinMode(Buzz, INPUT);
}

void busted(){ // Interrupt function
  if(!nono){
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    if(interrupt_time - last_interrupt_time > 50){ // Debounce / delay 
      failed_no++;
      failed = true;
    }
    last_interrupt_time = interrupt_time;
  }
}

void loop() {

  newStart = false;
  flashHighScores = 0;

  while(digitalRead(StartPad1)==HIGH && digitalRead(StartPad2)==HIGH){ 
    flashHighScores++;
    if(flashHighScores == 1){
     ReadHighscore(10);
     lc.setDigit(0,7,1,true);
     displayTime(Score);
    }
    else if(flashHighScores == 30){
      ReadHighscore(30);
      lc.setDigit(0,7,2,true);
      displayTime(Score);
    }
    else if(flashHighScores == 60){
      ReadHighscore(50);
      lc.setDigit(0,7,3,true);
      displayTime(Score);
    }
    else if(flashHighScores == 90)
      flashHighScores=0;
  
    delay(100);
  }
 
  delay(200);
  lc.clearDisplay(0);
  
  if(digitalRead(StartPad1)==LOW){
    displayTime(0);
    while(digitalRead(StartPad1)==LOW){
      delay(100);
    }
    nono = true;
    attachInterrupt(WireIO, busted, CHANGE);
    Start = millis();
    nono = false;
    while(digitalRead(StartPad2) == HIGH && !newStart && (failed_no < 3)){
      Elapsed=millis() - Start; 
      displayTime(Elapsed);
      if(Elapsed > MaxTime)
        failed_no = 3;
      if(digitalRead(StartPad1) == LOW)
        newStart = true;
      if(failed){
        warningSound();
        failed = false;
        if(failed_no == 1)
          digitalWrite(Led1, HIGH);
        if(failed_no == 2){
          digitalWrite(Led1, HIGH);
          digitalWrite(Led2, HIGH);
        }
      }
    }
  }
  
  else if(digitalRead(StartPad2)==LOW){
    displayTime(0);
    while(digitalRead(StartPad2)==LOW){
      delay(100);
    }
    nono = true;
    attachInterrupt(WireIO, busted, CHANGE); // Interrupt
    Start = millis();
    nono = false;
    while(digitalRead(StartPad1) == HIGH && !newStart && (failed_no < 3)){
      Elapsed=millis() - Start; 
      displayTime(Elapsed);
      if(Elapsed > MaxTime)
        failed_no = 3;
      if(digitalRead(StartPad2) == LOW)
        newStart = true;
      if(failed){
        warningSound();
        failed = false;
        if(failed_no == 1)
          digitalWrite(Led1, HIGH);
        if(failed_no == 2){
          digitalWrite(Led1, HIGH);
          digitalWrite(Led2, HIGH);
        }
      }
    }
  }

  detachInterrupt(WireIO);
  
  if(failed_no > 2){
    lc.clearDisplay(0);
    failed_no = 0;
    digitalWrite(Led3, HIGH);
    dispLoser();
    failedSound();
    digitalWrite(Led1, LOW);
    digitalWrite(Led2, LOW);
    digitalWrite(Led3, LOW);
    delay(500);
    digitalWrite(Led1, HIGH);
    digitalWrite(Led2, HIGH);
    digitalWrite(Led3, HIGH);
    delay(500);
    digitalWrite(Led1, LOW);
    digitalWrite(Led2, LOW);
    digitalWrite(Led3, LOW);
    delay(500);
    digitalWrite(Led1, HIGH);
    digitalWrite(Led2, HIGH);
    digitalWrite(Led3, HIGH);
    delay(500);
    digitalWrite(Led1, LOW);
    digitalWrite(Led2, LOW);
    digitalWrite(Led3, LOW);
    lc.clearDisplay(0);
  }
  else if(newStart)
    newStart = false;
  else if(Elapsed > 0){
    listePlass = CheckScore(Elapsed);
    successSound();
    if(listePlass < 4){
      dispPlace(listePlass);
      InsertScore(Elapsed);
    }
    else{
      delay(1500);
      dispNoRec();
    }
    digitalWrite(Led1, LOW);
    digitalWrite(Led2, LOW);
    digitalWrite(Led3, LOW);
  }

  Elapsed=0; 
  displayTime(Elapsed);
  failed_no = 0;
  failed = false;
  digitalWrite(Led1, LOW);
  digitalWrite(Led2, LOW);
  digitalWrite(Led3, LOW);
}
