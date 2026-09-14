#include <Arduino.h>
#include <ESP32Servo.h>

int Light = 23; // LED is wired to GPIO23 through a 220ohm resistor (see diagram.json)
int ServoPin = 19; // Servo PWM is wired to GPIO19 (see diagram.json)

Servo myServo;

void setup() {
  pinMode(Light, OUTPUT);

  myServo.attach(ServoPin);
}

void loop() {
  digitalWrite(Light, HIGH);
  delay(500);
  digitalWrite(Light, LOW);
  delay(500); // this speeds up the simulation

  // Sweep 0 -> 180 -> 0 to confirm the servo moves
  for (int angle = 0; angle <= 180; angle += 5) {
    myServo.write(angle);
    delay(15);
  }
  for (int angle = 180; angle >= 0; angle -= 5) {
    myServo.write(angle);
    delay(15);
  }
}
