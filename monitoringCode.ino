/************ LIBRARIES ************/
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

/************ LCD & ESP ************/
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
SoftwareSerial esp(3, 2);   // RX, TX

/************ PINS ************/
const int trigPin = A1;
const int echoPin = A2;
const int fan = 7;
const int buzzer = 6;

/************ VARIABLES ************/
long duration;
int distanceCm;
int temp;
int tempC;

/************ SETUP ************/
void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(fan, OUTPUT);
  pinMode(buzzer, OUTPUT);

  Serial.begin(9600);
  esp.begin(115200);

  lcd.begin(16, 2);
  lcd.print("TRANSFORMER");
  lcd.setCursor(0, 1);
  lcd.print("MONITORING");
  delay(2000);
  lcd.clear();
}

/************ LOOP ************/
void loop() {

  // Ultrasonic
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * 0.034 / 2;

  // Temperature
  temp = analogRead(A0);
  tempC = map(temp, 0, 1023, 0, 100);

  lcd.setCursor(0, 0);
  lcd.print("TEMP: ");
  lcd.print(tempC);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("OIL LVL: ");
  lcd.print(distanceCm);

  // ALERTS
  if (tempC > 35) {
    digitalWrite(buzzer, HIGH);
    digitalWrite(fan, HIGH);
  } else {
    digitalWrite(buzzer, LOW);
    digitalWrite(fan, LOW);
  }

  sendData(distanceCm, tempC);
  delay(2000);
}

/************ SEND DATA ************/
void sendData(int oil, int temp) {
  esp.print("OIL=");
  esp.print(oil);
  esp.print("&TEMP=");
  esp.println(temp);
}
