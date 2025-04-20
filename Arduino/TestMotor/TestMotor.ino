#include "fsm.h"
#include "motor.h"
#include "config.h"

// the setup routine runs once when you press reset:
void setup() {

  // initialize serial communication
  fsm_setup();
  motor_setup();
}

// the loop routine runs over and over again forever:
void loop() {
  if (digitalRead(PIN_USWITCH) == LOW) {
    digitalWrite(PIN_DAY_STATE, HIGH);

    motor_stop();
    delay(2000);
    motor_start(Direction::Down);
    delay(1000);

  } else {
    digitalWrite(PIN_DAY_STATE, LOW);
  }


  if (digitalRead(PIN_BSWITCH) == LOW) {
    digitalWrite(PIN_ERROR_STATE, HIGH);

    motor_stop();
    delay(2000);
    motor_start(Direction::Up);
    delay(1000);

  } else {
    digitalWrite(PIN_ERROR_STATE, LOW);
  }

}
