#include "motor.h"
#include "config.h"

// Private stuff -----------------------------------------------------------------------------

Direction dir;
uint8_t pwm; // Current pwm value, might be in ramp up/down

void setDutyPercent(uint8_t percent) {

  uint8_t pwmValue = (percent * 255) / 100;
  analogWrite(PIN_MOTOR_PWM, pwmValue);

}

// Public stuff -----------------------------------------------------------------------------

void motor_setup() {
  // Output to control PWM
  pinMode(PIN_MOTOR_PWM, OUTPUT);
  pinMode(PIN_MOTOR_DIR, OUTPUT);

  dir = Direction::Down;
  pwm = 0;
}

void motor_start(Direction d) {
  dir = d;
  if (dir == Direction::Up) 
  {
    digitalWrite(PIN_MOTOR_DIR, 1);
  }
  else 
  {
    digitalWrite(PIN_MOTOR_DIR, 0);
  }
  while (pwm < MAX_MOTOR_SPEED) {
    pwm += 1;
    setDutyPercent(pwm);
    delay(PWM_DELAY_MS); // total ramp of 1s
  }
}


void motor_stop() {

  while (pwm > 0) {
    pwm -= 1;
    setDutyPercent(pwm);
    delay(PWM_DELAY_MS); // total ramp of 1s
  }
}


