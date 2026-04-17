#include <Wire.h>
 
#include <LiquidCrystal.h>
 
#include <DHT.h>
 
#include <SevSeg.h>
 
LiquidCrystal lcd(22, 23, 24, 25, 26, 27);
 
#define DHTPIN 28
 
#define DHTTYPE DHT11
 
DHT dht(DHTPIN, DHTTYPE);
 
SevSeg sevseg;
 
// I2C
 
int receivedTempRaw = 235;
 
float preferredTemp = 23.5;
 
float activePreferredTemp = 23.5;
 
// Siren
 
static int freq = 1000;
 
static bool up = true;
 
// PIR
 
const int pirPin = 29;
 
// Ultrasonic
 
const int trigPin = 30;
 
const int echoPin = 31;
 
// Motor and buzzer
 
const int motorPin = 2;
 
const int buzzerPin = 43;
 
// LDR
 
const int ldrPin = A0;
 
// LEDs
 
const int yellowLeds[4] = {48, 49, 50, 51};
 
const int redLeds[4] = {44, 45, 46, 47};
 
// Motion counter
 
int motionCount = 0;
 
bool lastMotionState = LOW;
 
// 4-digit display timer
 
unsigned long motorRunStart = 0;
 
unsigned long totalMotorRunMs = 0;
 
bool lastMotorState = false;
 
void setup() {
 
  Wire.begin(8);
 
  Wire.onReceive(receiveEvent);
 
  lcd.begin(16, 2);
 
  dht.begin();
 
  pinMode(pirPin, INPUT);
 
  pinMode(trigPin, OUTPUT);
 
  pinMode(echoPin, INPUT);
 
  pinMode(motorPin, OUTPUT);
 
  digitalWrite(motorPin, LOW);
 
  pinMode(buzzerPin, OUTPUT);
 
  noTone(buzzerPin);
 
  for (int i = 0; i < 4; i++) {
 
    pinMode(yellowLeds[i], OUTPUT);
 
    pinMode(redLeds[i], OUTPUT);
 
    digitalWrite(yellowLeds[i], LOW);
 
    digitalWrite(redLeds[i], LOW);
 
  }
 
  // 4-digit display setup
 
  byte numDigits = 4;
 
  byte digitPins[] = {10, 11, 12, 13};
 
  byte segmentPins[] = {4, 5, 6, 7, 8, 9, 38, 39};
 
  bool resistorsOnSegments = true;
 
  byte hardwareConfig = COMMON_CATHODE;
 
  bool updateWithDelays = false;
 
  bool leadingZeros = true;
 
  bool disableDecPoint = true;
 
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins,
 
               resistorsOnSegments, updateWithDelays, leadingZeros, disableDecPoint);
 
  sevseg.setBrightness(90);
 
  sevseg.setNumber(0, 0);
 
  Serial.begin(9600);
 
}
 
void loop() {
 
  int motion = digitalRead(pirPin);
 
  int lightValue = analogRead(ldrPin);
 
  bool nightMode = (lightValue < 800);
 
  // Motion counter
 
  if (motion == HIGH && lastMotionState == LOW) {
 
    motionCount++;
 
  }
 
  lastMotionState = motion;
 
  // Night mode / preferred temp
 
    activePreferredTemp = preferredTemp;
 
 
 
  float t = dht.readTemperature();
 
  float h = dht.readHumidity();
 
  float distance = getDistance();
 
  bool danger = (distance > 0 && distance < 20);
 
  bool motorOn = false;
 
  // Yellow LEDs = motion
 
for (int i = 0; i < 4; i++) {
 
  digitalWrite(yellowLeds[i], danger ? HIGH : LOW);
 
}
 
  // Red LEDs = night mode
 
  for (int i = 0; i < 4; i++) {
 
    digitalWrite(redLeds[i], nightMode ? HIGH : LOW);
 
  }
 
  // DHT error handling
 
  if (isnan(t) || isnan(h)) {
 
    lcd.clear();
 
    lcd.setCursor(0, 0);
 
    lcd.print("DHT Error");
 
    lcd.setCursor(0, 1);
 
    lcd.print("Check Sensor");
 
    digitalWrite(motorPin, LOW);
 
    noTone(buzzerPin);
 
    unsigned long errorStart = millis();
 
    while (millis() - errorStart < 1000) {
 
      sevseg.refreshDisplay();
 
    }
 
    return;
 
  }
 
  // Ultrasonic safety + motor logic
 
  if (danger) {
 
    digitalWrite(motorPin, LOW);
 
    motorOn = false;
 
    // COP SIREN only when object is close
 
    if (up) {
 
      freq += 30;
 
      if (freq >= 2000) up = false;
 
    } else {
 
      freq -= 30;
 
      if (freq <= 1000) up = true;
 
    }
 
    tone(buzzerPin, freq);
 
  }
 
  else {
 
    noTone(buzzerPin);
 
    if (distance >= 20) {
 
      digitalWrite(motorPin, HIGH);
 
      motorOn = true;
 
    } else {
 
      digitalWrite(motorPin, LOW);
 
      motorOn = false;
 
    }
 
  }
 
  // Motor run-time tracking
 
  if (motorOn && !lastMotorState) {
 
    motorRunStart = millis();
 
  }
 
  if (!motorOn && lastMotorState) {
 
    totalMotorRunMs += millis() - motorRunStart;
 
  }
 
  unsigned long currentRunMs = totalMotorRunMs;
 
  if (motorOn) {
 
    currentRunMs += millis() - motorRunStart;
 
  }
 
  int runSeconds = currentRunMs / 1000;
 
  if (runSeconds > 9999) runSeconds = 9999;
 
  sevseg.setNumber(runSeconds, 0);
 
  lastMotorState = motorOn;
 
  // LCD display
 
  lcd.clear();
 
  if (nightMode) {
 
    lcd.setCursor(0, 0);
 
    lcd.print("Night Mode");
 
    lcd.setCursor(0, 1);
 
    lcd.print("SYS:");
 
    lcd.print(danger ? "STOP" : "RUN ");
 
    lcd.print(" C:");
 
    lcd.print(motionCount);
 
  }
 
  else if (danger) {
 
    lcd.setCursor(0, 0);
 
    lcd.print("SAFE STOP");
 
    lcd.setCursor(0, 1);
 
    lcd.print("D:");
 
    lcd.print(distance, 0);
 
    lcd.print("cm");
 
    lcd.print(" C:");
 
    lcd.print(motionCount);
 
  }
 
  else {
 
    lcd.setCursor(0, 0);
 
    lcd.print("T:");
 
    lcd.print(t, 1);
 
    lcd.print((char)223);
 
    lcd.print("C ");
 
    lcd.print("H:");
 
    lcd.print(h, 0);
 
    lcd.print("%");
 
    lcd.setCursor(0, 1);
 
    lcd.print("SYS:");
 
    lcd.print(motorOn ? "RUN " : "IDLE");
 
    lcd.print(" C:");
 
    lcd.print(motionCount);
 
  }
 
 
  // Serial output
 
  Serial.print("Temp:");
 
  Serial.print(t, 1);
 
  Serial.print(",Hum:");
 
  Serial.print(h, 1);
 
  Serial.print(",Motor:");
 
  Serial.print(motorOn ? "ON" : "OFF");
 
  Serial.print(",Night:");
 
  Serial.print(nightMode ? "ON" : "OFF");
 
  Serial.print(",Motion:");
 
  Serial.print(motion == HIGH ? "YES" : "NO");
 
  Serial.print(",Count:");
 
  Serial.print(motionCount);
 
  Serial.print(",Dist:");
 
  Serial.print(distance, 1);
 
  Serial.print(",Pref:");
 
  Serial.println(activePreferredTemp, 1);
 
  unsigned long refreshStart = millis();
 
  while (millis() - refreshStart < 500) {
 
    sevseg.refreshDisplay();
 
  }
 
}
 
void receiveEvent(int howMany) {
 
  if (Wire.available() >= 2) {
 
    byte highPart = Wire.read();
 
    byte lowPart = Wire.read();
 
    receivedTempRaw = word(highPart, lowPart);
 
    preferredTemp = receivedTempRaw / 10.0;
   
  }
 
}
 
 
float getDistance() {
 
  long duration;
 
  float distance;
 
  digitalWrite(trigPin, LOW);
 
  delayMicroseconds(2);
 
  digitalWrite(trigPin, HIGH);
 
  delayMicroseconds(10);
 
  digitalWrite(trigPin, LOW);
 
  duration = pulseIn(echoPin, HIGH, 30000);
 
  if (duration == 0) {
 
    return -1;
 
  }
 
  distance = duration * 0.034 / 2.0;
 
  return distance;
 
}