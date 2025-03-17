#include "motor.h"
#include "fsm.h"

// the setup routine runs once when you press reset:
void setup() {

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  Serial.println("Start setup");

  fsm_setup();
  motor_setup();

  Serial.println("Setup done");
}

// the loop routine runs over and over again forever:
void loop() {
  fsm_tick();
  delay(10);
}

#if 0

// Read light sensor
bool isDayNow() {
  int sensorValue = analogRead(A0);
  bool result = false;

  if (sensorValue < DAY_TRESHOLD) {
    if (dayCount == 0) {
      result = false;
    } else {
      result = true;
      dayCount--;
    }
  } else {
    if (dayCount > DAY_SEQ_COUNT) {
      result = true;
    } else {
      result = false;
      dayCount++;
    }
  }

  char strBuf[50];
  sprintf(strBuf, "I see (%d), dayCount=%d => %d", sensorValue, dayCount, result);
  Serial.println(strBuf);

  return result;
}

// Read upper sensor
bool isUpNow() {
  int sensorValue = analogRead(A2);
  char strBuf[50];
  sprintf(strBuf, "Sense up (%d)", sensorValue);
  Serial.println(strBuf);
  if (sensorValue > SWITCH_TRESHOLD) {
    return true;
  } else {
    return false;
  }
}
// Read lower sensor
bool isDownNow() {
  int sensorValue = analogRead(A1);
  char strBuf[50];
  sprintf(strBuf, "Sense up (%d)", sensorValue);
  Serial.println(strBuf);
  if (sensorValue > SWITCH_TRESHOLD) {
    return true;
  } else {
    return false;
  }
}

void openDoor() {
  digitalWrite(8, 1);

  // Read limit sensor, when high we are fully open
  while (!isUpNow()) {
    // Run the motor
   
  }

  // Stop
  digitalWrite(4, 0);
}

void closeDoor() {
  digitalWrite(8, 0);

  // Read limit sensor, when high we are fully closed
  while (!isDownNow()) {
    // Run the motor
    
  }


  // Stop
  digitalWrite(4, 0);
}

#endif
