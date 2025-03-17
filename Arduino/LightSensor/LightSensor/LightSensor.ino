//#include "ArduinoLowPower.h"

void setup() {
  // put your setup code here, to run once:
  pinMode(A0, INPUT);  // Light sensor

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

void loop() {

  int value = analogRead(A0);

  Serial.println(value);

  //delay(10*1000); // 2 sec
  sleep(5);
}

void sleep(int minutes) {

  for (int i = 0; i < 6; i++) 
  {
    for (int m = 0; m < minutes; m++) 
    {
      delay(10*1000); // 10 sec
    }
  }

}
