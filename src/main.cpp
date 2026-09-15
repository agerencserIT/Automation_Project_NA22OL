#include <Arduino.h>
#include <DHT.h>
#include <ESP32Servo.h>

#define LDR_PIN 34
#define LED_PIN 23
const int LIGHT_THRESHOLD = 2000;

#define DHTPIN 14
#define DHTTYPE DHT22

#define SERVO_PIN 19

DHT dht(DHTPIN, DHTTYPE);
Servo ventServo;

void setup() {

  pinMode(LDR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);

  dht.begin();

  ventServo.attach(SERVO_PIN);
  ventServo.write(0);

  Serial.println("Smart Room System Starting...");
}

void loop() {

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read DHT22!");
    delay(2000);
    return;
  }

  int lightLevel = analogRead(LDR_PIN);

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Light Level: ");
  Serial.println(lightLevel);

  if (lightLevel < LIGHT_THRESHOLD) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Light: ON");
  }
  else {
    digitalWrite(LED_PIN, LOW);
    Serial.println("Light: OFF");
  }

  if (temperature > 30) {
    ventServo.write(90);
    Serial.println("Ventilation: OPEN");
  }
  else if (temperature < 27) {
    ventServo.write(0);
    Serial.println("Ventilation: CLOSED");
  }

  Serial.println("--------------------");

  delay(2000);
}