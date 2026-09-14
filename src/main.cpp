#include <Arduino.h>
#include <ESP32Servo.h>

int Light = 23; // LED is wired to GPIO23 through a 220ohm resistor (see diagram.json)
int ServoPin = 19; // Servo PWM is wired to GPIO19 (see diagram.json)

Servo VentServo;

void setup() {
  pinMode(Light, OUTPUT);

  VentServo.attach(ServoPin);
}

void loop() {
  // Turn both systems to ON state
  VentServo.write(90);
  delay(1000);

  digitalWrite(Light, HIGH);
  delay(500);
 
  // Turn both systems to OFF state
  VentServo.write(0);
  delay(1000);

   digitalWrite(Light, LOW);
  delay(500);
}
