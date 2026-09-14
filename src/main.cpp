#include <Arduino.h>

int Light = 23; // LED is wired to GPIO23 through a 220ohm resistor (see diagram.json)

void setup() {
  pinMode(Light, OUTPUT);
}

void loop() {
  digitalWrite(Light, HIGH);
  delay(500);
  digitalWrite(Light, LOW);
  delay(500); // this speeds up the simulation
}
