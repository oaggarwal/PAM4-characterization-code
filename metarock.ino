#include <string.h>

const uint8_t LED = 13;

// LMK03806 pins
const uint8_t LMK_SDO = 2;
const uint8_t LMK_SCL = 3;
const uint8_t LMK_LATCH = 4;
const uint8_t LMK_SDI = 5;

// metarock pins

const uint8_t P_RST = 26;

const uint8_t P_CFGCLK = 30;

const uint8_t P_CFGIN = 38;

String inputString = "";         // A string to hold incoming data
boolean stringComplete = false;  // Whether the string is complete

const int MAX_PARTS = 5;
const int MAX_STRING_SIZE = 28;
char inputString_split[MAX_PARTS][MAX_STRING_SIZE];

void setup() {
  Serial.begin(115200);           // Starts the serial communication
  inputString.reserve(200);       // Reserve 200 bytes for the inputString

  pinMode(LED, OUTPUT);
  pinMode(LMK_SDO, INPUT);
  pinMode(LMK_SCL, OUTPUT);
  pinMode(LMK_LATCH, OUTPUT);
  pinMode(LMK_SDI, OUTPUT);

  
  pinMode(P_RST, OUTPUT);
  
  pinMode(P_CFGCLK, OUTPUT);
  
  pinMode(P_CFGIN, OUTPUT);

  
}

void loop() {


  if (stringComplete) {
    splitInputString();

    if (inputString.indexOf("blink") != -1) {
      blink();
      Serial.println("OK!");
    }

    if (inputString.indexOf("LMKWrite") != -1) {
      LMKWrite(strtoul(inputString_split[1], NULL, 10));
      Serial.println("OK!");
    }

    if (inputString.indexOf("LMKRead") != -1) {
      LMKRead(strtoul(inputString_split[1], NULL, 10));
    }

    if (inputString.indexOf("WriteConfig") != -1) {
      WriteConfig(strtoul(inputString_split[1], NULL, 10));
      Serial.println("OK!");
    }

    if (inputString.indexOf("WriteGPIO") != -1) {
      int pin_write = -1;
      if (strcmp(inputString_split[1], "RST") == 0) pin_write = P_RST;
      if (strcmp(inputString_split[1], "CFGIN") == 0) pin_write = P_CFGIN;
  

      if (pin_write > -1) {
        if (inputString_split[2][0] == '1') {
          digitalWrite(pin_write, HIGH);
          
        } else if (inputString_split[2][0] == '0') {
          digitalWrite(pin_write, LOW);
          
        } else {
          digitalWrite(pin_write, LOW);
          digitalWrite(pin_write, HIGH);
          digitalWrite(pin_write, LOW);
          // digitalWrite(P_RST, HIGH);
        }
          checkRSTAndCFGCLK();
        // checkRSTAndCFGIN();
      }
      
      
      Serial.println("OK!");
    }

  

    // Clear the string for new input:
    inputString = "";
    stringComplete = false;
  }
}

void checkRSTAndCFGCLK() {
  // If RST is low, force CFGCLK to low
  if (digitalRead(P_RST) == 0) {
    digitalWrite(P_CFGCLK, 0);
  }
}

// void checkRSTAndCFGIN() {
//   // If RST is low, force CFGCLK to low
//   if (digitalRead(P_RST) == 0) {
//     digitalWrite(P_CFGIN, 0);
//   }
// }


void WriteConfig(uint32_t value) { 
  // digitalWrite(P_RST, HIGH); // Set RST HIGH DURING WriteConfig

  // for (int i = 0; i < 5; i++) {
    for (int i = 0; i < 28; i++) {
      digitalWrite(P_CFGIN, (value & (1UL << i)) ? HIGH : LOW);
      digitalWrite(P_CFGCLK, HIGH);
      digitalWrite(P_CFGCLK, LOW);
    }
    
  // }


  digitalWrite(P_CFGCLK, LOW);
  digitalWrite(P_CFGIN, HIGH);

}
 

void LMKRead(uint32_t regis) { // check if this is configd correctly for clockgroup2
  uint32_t value = 0;
  uint32_t R31 = (0 << 21) | (regis << 16) | (1 << 5) | 31;
  LMKWrite(R31);
  for (int i = 0; i < 32; i++) {
    digitalWrite(LMK_SCL, HIGH);
    digitalWrite(LMK_SCL, LOW);
    value <<= 1;
    if (digitalRead(LMK_SDO) == HIGH && i < 27) {
      value |= 1;
    }
  }
  uint32_t value2 = value | regis;
  Serial.println(value2);
}

void LMKWrite(uint32_t value) {
  digitalWrite(LMK_SCL, LOW);
  digitalWrite(LMK_SDI, LOW);
  digitalWrite(LMK_LATCH, LOW);
  delay(1);

  for (int i = 31; i >= 0; i--) {
    digitalWrite(LMK_SDI, (value & (1UL << i)) ? HIGH : LOW);
    digitalWrite(LMK_SCL, HIGH);
    digitalWrite(LMK_SCL, LOW);
  }

  digitalWrite(LMK_SCL, LOW);
  digitalWrite(LMK_SDI, LOW);
  digitalWrite(LMK_LATCH, HIGH);
  digitalWrite(LMK_LATCH, LOW);
}

void blink() {
  digitalWrite(LED, HIGH);
  delay(10);
  digitalWrite(LED, LOW);
  delay(10);
}

void serialEvent() {  // check
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '>' || inChar == '\n') {
      stringComplete = true;
    } else if (inChar == '<') {
      stringComplete = false;
      inputString = "";
    } else {
      inputString += inChar;
    }
  }
}

void splitInputString() { // check
  memset(inputString_split, 0, sizeof(inputString_split));

  int prevIndex = 0;
  int partIndex = 0;

  while (partIndex < MAX_PARTS) {
    int separatorIndex = inputString.indexOf(":", prevIndex);
    String part = "";

    if (separatorIndex == -1) part = inputString.substring(prevIndex);
    else part = inputString.substring(prevIndex, separatorIndex);

    if (part.length() > MAX_STRING_SIZE - 1) part = part.substring(0, MAX_STRING_SIZE - 1);

    part.toCharArray(inputString_split[partIndex], part.length() + 1);

    if (separatorIndex == -1) break;

    prevIndex = separatorIndex + 1;
    partIndex++;
  }


}










