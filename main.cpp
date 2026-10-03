#include <Arduino.h> 
#include <DHT.h> // DHT22 temperature and humidity sensor 
#include <ESP32Servo.h> // PWM control for the vent servo 
 
// Light sensor (LDR) and the LED 
#define LDR_PIN 34 
#define LED_PIN 23 
const int LIGHT_THRESHOLD = 2000; 
 
// Temperature and humidity sensor 
#define DHTPIN 14 
#define DHTTYPE DHT22 
 
#define SERVO_PIN 19 
 
DHT dht(DHTPIN, DHTTYPE); 
Servo ventServo; 
 
void setup() { 
 
  pinMode(LDR_PIN, INPUT); 
  pinMode(LED_PIN, OUTPUT); 
 
  Serial.begin(115200); // Serial monitor for debugging output 
 
  dht.begin(); 
 
  // Start with the vent closed 
  ventServo.attach(SERVO_PIN); 
  ventServo.write(0); 
 
  Serial.println("Smart Room System Starting..."); 
} 
 
void loop() { 
 
  float temperature = dht.readTemperature(); 
  float humidity = dht.readHumidity(); 

  // A failed DHT22 read (e.g. a bad checksum) returns NaN, so skip this cycle 
  if (isnan(temperature) || isnan(humidity)) { 
    Serial.println("Failed to read DHT22!"); 
    digitalWrite(LED_PIN, LOW); 
    ventServo.write(90); 
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
 
  // Turn the LED on when the light reading drops below the threshold 
  if (lightLevel < LIGHT_THRESHOLD) { 
    digitalWrite(LED_PIN, HIGH); 
    Serial.println("Light: ON"); 
 } 
  else { 
    digitalWrite(LED_PIN, LOW); 
    Serial.println("Light: OFF"); 
  } 
    
  // Open when temperature is above 30°C and the humidity is above 70 
  if (temperature > 30 && humidity >= 70) { 
    ventServo.write(90); 
    Serial.println("Room Status: HOT & HUMID"); 
    Serial.println("Ventilation: FULLY OPEN"); 
 
    // add thingspeak warning here later 
  } 
 
  // Open when temperature is above 30°C and the humidity is below 70 
  else if (temperature > 30 && humidity < 70) { 
    ventServo.write(90); 
    Serial.println("Room Status: HOT"); 
    Serial.println("Ventilation: FULLY OPEN"); 
  } 
  // Half Open when temperature is below 27°C and below or equal to 30 
  else if (temperature >= 27 && temperature <= 30) { 
    ventServo.write(45); 
    Serial.println("Room Status: WARM"); 
    Serial.println("Ventilation: HALFWAY OPEN"); 
  } 
 
  // Otherwise close the vent 
  else { 
    ventServo.write(0); 
    Serial.println("Room Status: COMFORTABLE"); 
    Serial.println("Ventilation: CLOSED"); 
  } 
 
  Serial.println("--------------------"); 
 
  delay(2000); // Check the sensors every 2 seconds 
} 