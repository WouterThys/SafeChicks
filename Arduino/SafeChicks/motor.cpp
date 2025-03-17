#include "motor.h"
#include "pinout.h"

struct MotorState {
  Direction d;
  uint8_t pwm; // Current pwm value, might be in ramp up/down
};

MotorState state;

// Private stuff -----------------------------------------------------------------------------

void setDutyPercent(uint8_t percent) {

  uint8_t pwmValue = (percent * 255) / 100;
  analogWrite(PIN_MOTOR, pwmValue);

}

// Public stuff -----------------------------------------------------------------------------

void motor_setup() {
  // Output to control PWM
  pinMode(PIN_MOTOR, OUTPUT);

  state.d = Direction::Down;
  state.pwm = 0;
}

void motor_start(Direction d) {
  state.d = d;
  while (state.pwm < 100) {
    state.pwm += 1;
    setDutyPercent(state.pwm);
    delay(10); // total ramp of 1s
  }
}


void motor_stop() {

  while (state.pwm > 0) {
    state.pwm -= 1;
    setDutyPercent(state.pwm);
    delay(10); // total ramp of 1s
  }
}


