
#include "wiring_private.h"


#define Type1

#if defined(Type0)
const uint8_t X_PINS[] = {
  2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};
#else
const uint8_t X_PINS[] = {
  2, 3, 4, 5, 6, 7, 8, 9
};
#endif
const uint8_t X_PINS_LENGTH = sizeof(X_PINS) / sizeof(X_PINS[0]);

volatile uint8_t *yState;
volatile uint8_t *y2State;

const unsigned long ANTI_CHATTERING = 200; // 200ms

const char TABLE_DEC2HEX[] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};


void setup() {
  Serial.begin(9600);

  for (int8_t x = X_PINS_LENGTH - 1; x >= 0; x--) {
    pinMode(X_PINS[x], OUTPUT);
    digitalWrite(X_PINS[x], LOW);
  }

  yState = portInputRegister(digitalPinToPort(A0));
  y2State = portInputRegister(digitalPinToPort(11));

#if defined(ADCSRA)
  cbi(ADCSRA, ADPS2);
  sbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);
#endif

  checkPerformance();
}

void checkPerformance() {
  unsigned long time = millis();

  for (int i = 0; i < 1000; i++) {
    traversePins();
  }

  Serial.println(millis() - time);
}

void loop() {
  traversePins();
}

void traversePins() {
  for (int8_t x = X_PINS_LENGTH - 1; x >= 0; x--) {
    digitalWrite(X_PINS[x], HIGH);

    uint8_t state = *yState;
#if defined(Type0)
    if (analogRead(A6) > 64) {
      state |= 0x80;
    }
#else
    state |= (((*y2State) & 24) << 3);
#endif

    if (state != 0) {
      // Serial.println(state);
      showPosition(x, state);
    }

    digitalWrite(X_PINS[x], LOW);
  }
}

void showPosition(uint8_t x, uint8_t state) {
  uint8_t y;
  switch (state) {
  case 1:
    y = 0;
    break;
  case 2:
    y = 1;
    break;
  case 4:
    y = 2;
    break;
  case 8:
    y = 3;
    break;
  case 16:
    y = 4;
    break;
  case 32:
    y = 5;
    break;
  case 64:
    y = 6;
    break;
  case 128:
    y = 7;
    break;
  default:
    y = 255;
    break;
  }

  static unsigned long prevTime = 0;
  if (y != 255) {
    unsigned long time = millis();
    if (time - prevTime > ANTI_CHATTERING) {
//      Serial.print(TABLE_DEC2HEX[x]);
//      Serial.println(TABLE_DEC2HEX[y]);
      Serial.write(x * 16 + y);
    }
    prevTime = time;
  }
}
