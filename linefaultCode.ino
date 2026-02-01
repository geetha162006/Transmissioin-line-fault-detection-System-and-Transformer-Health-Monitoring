/************ LIBRARIES ************/
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <LiquidCrystal.h>

/************ LCD & SERIAL ************/
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
SoftwareSerial nodemcu(9, 10);   // RX, TX

/************ CONSTANTS ************/
int mVperAmp = 100;
double pickup_current = 0.55;

/************ VARIABLES ************/
double fault_current = 0;
double Voltage1, Voltage2, Voltage3, Voltage4;
double VRMS1, VRMS2, VRMS3, VRMS4;
double AmpsRMS1, AmpsRMS2, AmpsRMS3, AmpsRMS4;

int FT = 1;
int MAP = 0;

/************ SETUP ************/
void setup() {
  pinMode(7, OUTPUT);     // Relay / indicator
  pinMode(13, OUTPUT);    // Buzzer

  lcd.begin(16, 2);

  Serial.begin(9600);
  nodemcu.begin(115200);

  lcd.print("TRANSMISSION");
  lcd.setCursor(0, 1);
  lcd.print("LINE SYSTEM");
  delay(2000);
  lcd.clear();
}

/************ LOOP ************/
void loop() {

  Voltage1 = getVPP(A0);
  Voltage2 = getVPP(A1);
  Voltage3 = getVPP(A2);
  Voltage4 = getVPP(A3);

  VRMS1 = (Voltage1 / 2.0) * 0.707;
  VRMS2 = (Voltage2 / 2.0) * 0.707;
  VRMS3 = (Voltage3 / 2.0) * 0.707;
  VRMS4 = (Voltage4 / 2.0) * 0.707;

  AmpsRMS1 = (VRMS1 * 1000) / mVperAmp;
  AmpsRMS2 = (VRMS2 * 1000) / mVperAmp;
  AmpsRMS3 = (VRMS3 * 1000) / mVperAmp;
  AmpsRMS4 = (VRMS4 * 1000) / mVperAmp;

  lcd.clear();

  if ((AmpsRMS2 > pickup_current) &&
      (AmpsRMS3 > pickup_current) &&
      (AmpsRMS4 > pickup_current)) {

    tone(13, 1000);
    lcd.print("3 PHASE FAULT");
    FT = 2;
    fault_current = max(AmpsRMS2, max(AmpsRMS3, AmpsRMS4));

  } else if ((AmpsRMS2 > pickup_current) ||
             (AmpsRMS3 > pickup_current) ||
             (AmpsRMS4 > pickup_current)) {

    tone(13, 1000);

    if (AmpsRMS1 > 0.35) {
      lcd.print("L-G FAULT");
      FT = 5;
    } else {
      lcd.print("L-L FAULT");
      FT = 4;
    }

    fault_current = max(AmpsRMS2, max(AmpsRMS3, AmpsRMS4));
  } 
  else {
    noTone(13);
    lcd.print("NO FAULT");
    FT = 1;
    fault_current = 0;
  }

  mapDistance();
  sendStatus();

  delay(1500);
}

/************ MAP FAULT DISTANCE ************/
void mapDistance() {
  lcd.setCursor(0, 1);

  if (fault_current >= 1.10) {
    MAP = 2;
    lcd.print("DIST: 2 KM");
  } 
  else if (fault_current >= 0.83) {
    MAP = 4;
    lcd.print("DIST: 4 KM");
  } 
  else if (fault_current >= 0.72) {
    MAP = 6;
    lcd.print("DIST: 6 KM");
  } 
  else if (fault_current >= pickup_current) {
    MAP = 8;
    lcd.print("DIST: 8 KM");
  }
}

/************ SEND DATA TO NODEMCU ************/
void sendStatus() {
  StaticJsonDocument<256> doc;

  doc["FT"] = FT;
  doc["MAP"] = MAP;
  doc["I1"] = AmpsRMS1;
  doc["I2"] = AmpsRMS2;
  doc["I3"] = AmpsRMS3;

  serializeJson(doc, nodemcu);
}

/************ VOLTAGE PEAK TO PEAK ************/
float getVPP(int pin) {
  int maxValue = 0;
  int minValue = 1023;

  unsigned long start = millis();
  while (millis() - start < 1000) {
    int val = analogRead(pin);
    if (val > maxValue) maxValue = val;
    if (val < minValue) minValue = val;
  }

  return ((maxValue - minValue) * 5.0) / 1024.0;
}
