#include <Keypad.h>
#include <LiquidCrystal.h>
#include <Wire.h>

const byte mega2Address = 8;

// LCD
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Keypad
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {29, 28, 27, 26};
byte colPins[COLS] = {25, 24, 23, 22};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Buzzer
const int buzzerPin = 31;

// PB1
const int pb1Pin = 30;

// 74HC595 + 4-digit 7-segment
const int dataPin = 32;
const int clockPin = 33;
const int latchPin = 34;
const int digitPins[4] = {35, 36, 37, 38};

// Common cathode codes
byte digits[10] = {
  B00111111,
  B00000110,
  B01011011,
  B01001111,
  B01100110,
  B01101101,
  B01111101,
  B00000111,
  B01111111,
  B01101111
};

// Stages
// 1 = first pass
// 2 = second pass
// 3 = temp input
// 4 = wait for PB1
// 5 = system on
int stage = 1;

String pass1 = "";
String pass2 = "";
String tempInput = "";

float preferredTempC = 0.0;
float preferredTempF = 0.0;

void setup() {
  Wire.begin();

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Enter Pass:");
  lcd.setCursor(0, 1);
  lcd.print("____");

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  // use internal pull-up
  pinMode(pb1Pin, INPUT_PULLUP);

  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);

  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], HIGH);
  }

  clearDisplay();
}

void loop() {
  if (stage == 4) {
    waitForPB1();
    return;
  }

  char key = keypad.getKey();

  if (key) {
    if (stage == 1) {
      handleFirstPass(key);
    }
    else if (stage == 2) {
      handleSecondPass(key);
    }
    else if (stage == 3) {
      handleTempInput(key);
    }
  }
}

void handleFirstPass(char key) {
  if (key == '*') {
    pass1 = "";
    lcd.clear();
    lcd.print("Enter Pass:");
    lcd.setCursor(0, 1);
    lcd.print("____");
  }
  else if (key == '#') {
    if (pass1.length() == 4) {
      stage = 2;
      lcd.clear();
      lcd.print("Re-enter Pass:");
      lcd.setCursor(0, 1);
      lcd.print("____");
    }
  }
  else if (key >= '0' && key <= '9') {
    if (pass1.length() < 4) {
      pass1 += key;
      lcd.setCursor(0, 1);
      lcd.print("____");
      lcd.setCursor(0, 1);
      lcd.print(pass1);
    }
  }
}

void handleSecondPass(char key) {
  if (key == '*') {
    pass2 = "";
    lcd.clear();
    lcd.print("Re-enter Pass:");
    lcd.setCursor(0, 1);
    lcd.print("____");
  }
  else if (key == '#') {
    if (pass2.length() == 4) {
      if (pass1 == pass2) {
        lcd.clear();
        lcd.print("Matched!");
        delay(1000);

        digitalWrite(buzzerPin, HIGH);
        delay(1000);
        digitalWrite(buzzerPin, LOW);

        flashDisplay(pass1.toInt());
        clearDisplay();

        stage = 3;
        tempInput = "";

        lcd.clear();
        lcd.print("Set Temp:");
        lcd.setCursor(0, 1);
        lcd.print("___");
      }
      else {
        lcd.clear();
        lcd.print("Mismatch!");
        delay(2000);

        pass1 = "";
        pass2 = "";
        stage = 1;

        lcd.clear();
        lcd.print("Enter Pass:");
        lcd.setCursor(0, 1);
        lcd.print("____");
      }
    }
  }
  else if (key >= '0' && key <= '9') {
    if (pass2.length() < 4) {
      pass2 += key;
      lcd.setCursor(0, 1);
      lcd.print("____");
      lcd.setCursor(0, 1);
      lcd.print(pass2);
    }
  }
}

void handleTempInput(char key) {
  if (key == '*') {
    tempInput = "";
    lcd.clear();
    lcd.print("Set Temp:");
    lcd.setCursor(0, 1);
    lcd.print("___");
  }
  else if (key == '#') {
    if (tempInput.length() == 3) {
      int rawValue = tempInput.toInt();
      preferredTempC = rawValue / 10.0;
      preferredTempF = (preferredTempC * 9.0 / 5.0) + 32.0;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(preferredTempC, 1);
      lcd.print((char)223);
      lcd.print("C");

      lcd.setCursor(0, 1);
      lcd.print(preferredTempF, 1);
      lcd.print((char)223);
      lcd.print("F");

      delay(3000);

      stage = 4;
      lcd.clear();
      lcd.print("Press PB1");
      lcd.setCursor(0, 1);
      lcd.print("to Start");
    }
  }
  else if (key >= '0' && key <= '9') {
    if (tempInput.length() < 3) {
      tempInput += key;

      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);

      if (tempInput.length() == 1) {
        lcd.print(tempInput[0]);
      }
      else if (tempInput.length() == 2) {
        lcd.print(tempInput[0]);
        lcd.print(tempInput[1]);
      }
      else if (tempInput.length() == 3) {
        lcd.print(tempInput[0]);
        lcd.print(tempInput[1]);
        lcd.print(".");
        lcd.print(tempInput[2]);
        lcd.print((char)223);
        lcd.print("C");
      }
    }
  }
}

void waitForPB1() {
  Serial.begin(9600);
  if (digitalRead(pb1Pin) == LOW) {
    lcd.clear();
    lcd.print("System is ON");

    int tempToSend = tempInput.toInt();   // example: 300 = 30.0C

    Wire.beginTransmission(mega2Address);
    Wire.write(highByte(tempToSend));
    Wire.write(lowByte(tempToSend));
    Wire.endTransmission();

    stage = 5;
  }
}

void flashDisplay(int num) {
  unsigned long startTime = millis();

  while (millis() - startTime < 5000) {
    unsigned long onStart = millis();
    while (millis() - onStart < 500) {
      displayNumber(num);
    }

    clearDisplay();
    delay(500);
  }
}

void displayNumber(int num) {
  int thousands = num / 1000;
  int hundreds = (num / 100) % 10;
  int tens = (num / 10) % 10;
  int ones = num % 10;

  showDigit(0, thousands);
  showDigit(1, hundreds);
  showDigit(2, tens);
  showDigit(3, ones);
}

void showDigit(int digitIndex, int value) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(digitPins[i], HIGH);
  }

  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, digits[value]);
  digitalWrite(latchPin, HIGH);

  digitalWrite(digitPins[digitIndex], LOW);
  delay(3);
  digitalWrite(digitPins[digitIndex], HIGH);
}

void clearDisplay() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(digitPins[i], HIGH);
  }

  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, 0);
  digitalWrite(latchPin, HIGH);
}