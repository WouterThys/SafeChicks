#include "config.h"



void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_DAY_STATE, OUTPUT);
  pinMode(PIN_ERROR_STATE, OUTPUT);

  pinMode(PIN_USWITCH, INPUT);
  pinMode(PIN_BSWITCH, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:

  if (digitalRead(PIN_USWITCH) == LOW) {
    digitalWrite(PIN_DAY_STATE, HIGH);
  } else {
    digitalWrite(PIN_DAY_STATE, LOW);
  }


  if (digitalRead(PIN_BSWITCH) == LOW) {
    digitalWrite(PIN_ERROR_STATE, HIGH);
  } else {
    digitalWrite(PIN_ERROR_STATE, LOW);
  }
}
