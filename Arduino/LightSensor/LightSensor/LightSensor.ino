//#include "ArduinoLowPower.h"

void setup() {
  // put your setup code here, to run once:
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

void loop() {

  int value0 = analogRead(A0);
  int value1 = analogRead(A1);
  int value2 = analogRead(A2);

  Serial.println(value0);
  Serial.println(value1);
  Serial.println(value2);
  Serial.println();

  delay(2*1000); // 2 sec
  //sleep(5);
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
