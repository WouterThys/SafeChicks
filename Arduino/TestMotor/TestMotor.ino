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
  fsm_tick();
}
