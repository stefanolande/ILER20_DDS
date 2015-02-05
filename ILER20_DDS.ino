#include <LiquidCrystal.h>
#include "EF_AD9850.h"
#include <EEPROM.h>
#include "SimpleTimer.h"

#define IF 3276000
#define OFFSET 450

/*COLLEGAMENTI DEL MODULO DDS ALL'ARDUINO*/
#define CLK  8
#define FQ   9
#define DATA 10
#define RST  11

//encoder
#define encoderPinA 2
#define encoderPinB 3
#define STEP 100
#define ENC_DELAY 50

#define MEMORY_UPD_INT 10000

#define DEBUG false

boolean A_set = false;
boolean B_set = false;


// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(13, 12, 7, 6, 5, 4);

EF_AD9850 AD9850(CLK, FQ, RST, DATA);

int freqi, freqd, i;
long frq = 14200000;
long lastFrq = 0;
char txtstr[100];
String str;

bool frqchange = 1;

SimpleTimer memTimer;

void setup() {

  Serial.begin(9600);

#if DEBUG
  while(!Serial);
#endif

  initEncoder();
  AD9850.init();
  AD9850.reset();
  readFreq();
  AD9850.wr_serial(0x00, frq - IF + OFFSET);

  lcd.begin(16, 2);
  lcd.print("    IS0EIR");
  lcd.setCursor(0, 1);
  lcd.print("    ILER-20");


  delay(3000);

  memTimer.setInterval(MEMORY_UPD_INT, saveFreq);

  updateFreq();


}

void loop() {

  memTimer.run();

  if (frqchange)
    updateFreq();


  checkSerial();

}

// Interrupt on A changing state
void doEncoderA() {
  // Test transition
  A_set = digitalRead(encoderPinA) == HIGH;
  // and adjust counter + if A leads B
  if (A_set != B_set){
    if(frq <= 14350000){
      frq += STEP; 
    }
  } 
  else{
    if (frq >= 14000000){
      frq -= STEP; 
    }
  }


  frqchange = 1;
  delay(ENC_DELAY);

}

// Interrupt on B changing state
void doEncoderB() {

  // Test transition
  B_set = digitalRead(encoderPinB) == HIGH;
  // and adjust counter + if B follows A
  if (A_set == B_set){
    if(frq <= 14350000){
      frq += STEP; 
    }
  } 
  else{
    if (frq >= 14000000){
      frq -= STEP; 
    }
  }

  frqchange = 1;
  delay(ENC_DELAY);

}

void updateFreq() {
  lcd.clear();
  lcd.setCursor(0, 1);
  freqi = frq / 1000000;
  freqd = (frq - freqi * 1000000) / 100;
  String freqdString = String(freqd);

  if (freqd < 1000){
    freqdString = "0" + freqdString;
  } 
  if (freqd < 100){
    freqdString = "0" + freqdString;
  }

  str = String(freqi) + "." + freqdString.substring(0, 3) + "." + freqdString.substring(3) + " MHz";


  lcd.print(str);


  AD9850.wr_serial(0x00, frq - IF + OFFSET);

#if DEBUG
  Serial.println(frq);
#endif

  frqchange = 0;
}

void initEncoder() {
  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);
  digitalWrite(encoderPinA, HIGH);  // turn on pullup resistor
  digitalWrite(encoderPinB, HIGH);  // turn on pullup resistor

  // encoder pin on interrupt 0 (pin 2)
  attachInterrupt(0, doEncoderA, CHANGE);
  // encoder pin on interrupt 1 (pin 3)
  attachInterrupt(1, doEncoderB, CHANGE);
}


void saveFreq(){
  if(frq != lastFrq){

#if DEBUG
    Serial.println("WRITE");
#endif

    String frqString;
    frqString = String(frq);

    for(int i=0; i < 8; i++){

      EEPROM.write(i, String(frqString.charAt(i)).toInt());
#if DEBUG
      Serial.println(String(i) + ", " + frqString.charAt(i));
#endif
    }
    lastFrq = frq;
  }
}

void readFreq(){

#if DEBUG
  Serial.println("READ");
#endif

  String frqString = "";

  for(int i=0; i<8; i++){
    frqString += EEPROM.read(i);
#if DEBUG
    Serial.println(String(i) + ", " + EEPROM.read(i));
#endif
  }

  frq = frqString.toInt(); 

  lastFrq = frq;
}

void checkSerial(){
  char c;
  double recFrq;
  if(Serial){
    // verifica la presenza di messaggi
    if (Serial.available() > 0) {
      c=Serial.read();
      
      
      
      switch(c){
      case 'F':
      case 'f':
        recFrq = Serial.parseInt();
        if( recFrq >= 14000000 && recFrq <= 14350000){
          frq = recFrq;
          frqchange = true;
        }
        break;
      case 'R':
      case 'r':
        Serial.println("\nR" + String(frq));
        break;
      default:
        lcd.setCursor(0, 0);
        lcd.print(7c);
        break;
      }

    } 
  }
}








