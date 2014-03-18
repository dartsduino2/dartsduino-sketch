
#include "wiring_private.h"

// #define BoardType0
#define BoardType1

// #define DEBUG_MODE

const uint8_t OUTPUT_PINS[] = {
#ifdef BoardType0
  2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
#else
  2, 3, 4, 5, 6, 7, 8, 9
#endif
};
const uint8_t OUTPUT_PINS_LENGTH = sizeof(OUTPUT_PINS) / sizeof(OUTPUT_PINS[0]);

volatile uint8_t *inputRegister;
volatile uint8_t *inputRegister2;

const unsigned long ANTI_CHATTERING_TIME = 200; // [ms]

const char TABLE_DEC2HEX[] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};


void setup() {
  Serial.begin(9600);

  for (int8_t i = OUTPUT_PINS_LENGTH - 1; i >= 0; i--) {
    pinMode(OUTPUT_PINS[i], OUTPUT);
    digitalWrite(OUTPUT_PINS[i], LOW);
  }

  setupPorts();

#ifdef DEBUG_MODE
  checkPerformance();
#endif
}

void setupPorts() {
  inputRegister  = portInputRegister(digitalPinToPort(A0));

#ifdef BoardType1
  inputRegister2 = portInputRegister(digitalPinToPort(11));

  cbi(ADCSRA, ADPS2);
  sbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);
#endif
}

#ifdef DEBUG_MODE
void checkPerformance() {
  unsigned long startTime = millis();
  for (int i = 0; i < 1000; i++) {
    traversePins();
  }
  unsigned long elapsedTime = millis() - startTime;

  Serial.println(elapsedTime);
}
#endif

void loop() {
  traversePins();
}

void traversePins() {
  uint8_t state;

  for (int8_t i = OUTPUT_PINS_LENGTH - 1; i >= 0; i--) {
    digitalWrite(OUTPUT_PINS[i], HIGH);

    state = getInputState();
    if (state != 0) {
      showHitPosition(i, state);
    }

    digitalWrite(OUTPUT_PINS[i], LOW);
  }
}

inline uint8_t getInputState() {
#if defined(BoardType0)
  return *inputRegister | ((analogRead(A6) > 64) ? 0x80 : 0);
#else
  return *inputRegister | (((*inputRegister2) & 24) << 3);
#endif
}

void showHitPosition(uint8_t outputPinIndex, uint8_t inputState) {
  static unsigned long prevTime = 0;

  unsigned long currTime = millis();
  unsigned long elapsedTime = currTime - prevTime;
  prevTime = currTime;

  if (elapsedTime < ANTI_CHATTERING_TIME) {
    return;
  }

  uint8_t inputBit;
  switch (inputState) {
  case 1:   inputBit = 0; break;
  case 2:   inputBit = 1; break;
  case 4:   inputBit = 2; break;
  case 8:   inputBit = 3; break;
  case 16:  inputBit = 4; break;
  case 32:  inputBit = 5; break;
  case 64:  inputBit = 6; break;
  case 128: inputBit = 7; break;
  default:  return;
  }

#ifdef DEBUG_MODE
  Serial.print(TABLE_DEC2HEX[outputPinIndex]);
  Serial.println(TABLE_DEC2HEX[inputBit]);
#else
  Serial.write((outputPinIndex << 4) + inputBit);
#endif
}
