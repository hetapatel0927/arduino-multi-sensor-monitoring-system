# Arduino Multi-Sensor Monitoring & Control System

## Overview
This project implements a real-time multi-sensor monitoring and control system using two Arduino MEGA boards. It integrates sensors, user input, and actuators to automate environmental monitoring and safety control.

The system is divided into:
- MEGA-1 → User Interface  
- MEGA-2 → Sensor Processing & Control  

A **detailed circuit wiring guide and step-by-step build instructions** are provided in the accompanying PDF report.

---

## System Architecture

User → Keypad → MEGA-1 → I2C → MEGA-2 → Sensors → Logic → Outputs

---

## Features

- Passcode authentication  
- Temperature input (decimal conversion)  
- I2C communication  
- PIR motion detection  
- Temperature & humidity (DHT11)  
- Ultrasonic safety system  
- Night mode (LDR)  
- LED indicators  
- Buzzer alerts  
- Motor control  
- 4-digit display timer  

---

## MEGA-1 (User Interface)

### Components
- Keypad  
- LCD (16x2)  
- Active buzzer  
- Push button (PB1)  
- 4-digit display (passcode flash)

### Responsibilities
- Passcode verification  
- Temperature input (e.g., `235 → 23.5°C`)  
- LCD display  
- Send temperature via I2C  

---

## MEGA-2 (Control Unit)

### Components
- PIR sensor  
- DHT11  
- Ultrasonic sensor  
- LDR  
- DC motor (PN2222 transistor)  
- Passive buzzer  
- Yellow LEDs  
- Red LEDs  
- 4-digit display (timer)

### Responsibilities
- Sensor processing  
- Motor control  
- Safety shutdown  
- Night mode  
- Timer display  

---

## Pin Configuration

### MEGA-1
- LCD → 12, 11, 5, 4, 3, 2  
- Keypad → 22–29  
- Buzzer → 31  
- PB1 → 30  
- 74HC595 → 32, 33, 34  

---

### MEGA-2
- PIR → 29  
- DHT11 → 28  
- Ultrasonic → 30, 31  
- Motor → 2  
- Buzzer → 43  
- LDR → A0  
- Yellow LEDs → 48–51  
- Red LEDs → 44–47  

---

### 4-Digit Display (Direct)
- Segments → 4–9, 38, 39  
- Digits → 10–13  

---

## I2C Connection

| MEGA-1 | MEGA-2 |
|--------|--------|
| SDA (20) | SDA (20) |
| SCL (21) | SCL (21) |
| GND | GND |

---

## Working Logic

### Temperature
Motor ON if:
Current Temperature > Preferred Temperature

---

### Motion
PIR HIGH:
- Yellow LEDs ON  

---

### Safety
Distance < 20 cm:
- Motor OFF  
- Buzzer ON  

---

### Night Mode
Low light detected:
- Red LEDs ON  
- Preferred temperature set to 22°C  

---

### Timer
Motor ON:
- Timer starts  
- Displays runtime in seconds  

---

## How to Build (Summary)

A complete step-by-step circuit build guide is provided in the PDF report. Summary:

1. Build MEGA-1:
   - Connect keypad, LCD, buzzer, and push button  
   - Test passcode and temperature input  

2. Build MEGA-2:
   - Connect DHT11, PIR, ultrasonic sensor  
   - Add motor with PN2222 transistor and diode  
   - Connect LEDs and buzzer  

3. Add LDR:
   - Create voltage divider using resistor  

4. Connect 4-digit display:
   - Direct wiring using segment and digit pins  

5. Connect I2C:
   - SDA → 20 to 20  
   - SCL → 21 to 21  
   - GND → GND  

6. Upload code:
   - Upload MEGA-1 code  
   - Upload MEGA-2 code  

7. Test system:
   - Enter passcode  
   - Set temperature  
   - Verify sensor responses  

---

## Testing

The following functionalities were verified:

- Passcode system  
- Temperature transmission  
- Sensor readings  
- Motor control  
- Safety shutdown  
- Night mode  
- Timer display  

---

## Sample Serial Output
Temp:24.5,Hum:60,Motor:ON,Night:OFF,Motion:YES,Dist:50,Pref:23.5

---

## Documentation

For complete details including:
- Full wiring diagrams  
- Exact pin-to-pin connections  
- Circuit build instructions  
- System flow explanation  

---
## Instruction Report
[Instruction](mega-report.pdf)

## Conclusion

This project demonstrates:
- Multi-sensor integration  
- Real-time embedded system design  
- I2C communication  
- Smart automation logic  

---

## Author
Heta Patel
