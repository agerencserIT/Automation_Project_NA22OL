#include <Arduino.h>
#include <DHT.h>         // DHT22 temperature and humidity sensor
#include <ESP32Servo.h>  // PWM control for the vent servo
#include <WiFi.h>
#include <HTTPClient.h>  // sends HTTP requests to ThingSpeak
#include "key.h"  // contains the ThingSpeak API key

// Light sensor (LDR) and the LED
#define LDR_PIN 34
#define LED_PIN 23
const int LIGHT_THRESHOLD = 2000;

// Temperature and humidity sensor
#define DHTPIN 14
#define DHTTYPE DHT22

#define SERVO_PIN 19

// Wokwi's simulated Wi-Fi network (no password)
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

const unsigned long UPLOAD_INTERVAL = 20000;  // free tier allows 1 update per 15s
unsigned long lastUpload = 0;

DHT dht(DHTPIN, DHTTYPE);
Servo ventServo;
int ventAngle = 0;  // 0 = closed, 45 = halfway, 90 = fully open

// Tries to connect to Wi-Fi for up to 10 seconds, then carries on either way
void connectWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(250);
    Serial.print(".");
  }
  Serial.println(WiFi.status() == WL_CONNECTED ? " connected" : " failed, will retry later");
}

// Sends the latest readings to ThingSpeak as a single HTTP GET request
void sendToThingSpeak(float temperature, float humidity, int lightLevel, bool ledOn, int ventAngle, bool hotHumidWarning) {
  // If Wi-Fi has dropped, skip this upload and reconnect in the background,
  // so the sensors, vent and light keep running
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi down, skipping upload");
    WiFi.reconnect();
    return;
  }

  // Build the request URL, with each value in its own ThingSpeak field
  String url = "http://api.thingspeak.com/update?api_key=" + String(THINGSPEAK_API_KEY) +
               "&field1=" + String(temperature) +
               "&field2=" + String(humidity) +
               "&field3=" + String(lightLevel) +
               "&field4=" + String(ledOn ? 1 : 0) +
               "&field5=" + String(ventAngle) +
               "&field6=" + String(hotHumidWarning ? 1 : 0);

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  // A 200 response returns the new entry number
  if (httpCode == 200) {
    Serial.println("ThingSpeak: entry " + http.getString());  // "0" means rejected
  } else {
    Serial.printf("ThingSpeak error: %d\n", httpCode);
  }
  http.end();  // close the connection
}

void setup() {
  pinMode(LDR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);  // serial monitor for debugging output

  dht.begin();

  // Start with the vent closed
  ventServo.attach(SERVO_PIN);
  ventServo.write(0);

  connectWiFi();

  Serial.println("Smart Room System Starting...");
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // A failed DHT22 read (e.g. a bad checksum) returns NaN. Open the vent fully and turn off the LED, then skip this cycle
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read DHT22!");
    digitalWrite(LED_PIN, LOW);
    ventAngle = 90;
    ventServo.write(ventAngle);
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
  bool ledOn = lightLevel < LIGHT_THRESHOLD;
  digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
  Serial.println(ledOn ? "Light: ON" : "Light: OFF");

  // Set the room status and vent position from the temperature and humidity
  const char* roomStatus;
  const char* ventStatus;
  bool hotHumidWarning = false;

  if (temperature > 30 && humidity >= 70) {
    roomStatus = "HOT & HUMID";
    ventAngle = 90;
    ventStatus = "FULLY OPEN";
    hotHumidWarning = true;
    Serial.println("WARNING: room is hot and humid");
    // add ThingSpeak warning here later
  }
  else if (temperature > 30) {
    roomStatus = "HOT";
    ventAngle = 90;
    ventStatus = "FULLY OPEN";
  }
  else if (temperature >= 27) {  // 27-30°C
    roomStatus = "WARM";
    ventAngle = 45;
    ventStatus = "HALFWAY OPEN";
  }
  else {
    roomStatus = "COMFORTABLE";
    ventAngle = 0;
    ventStatus = "CLOSED";
  }

  ventServo.write(ventAngle);
  Serial.print("Room Status: ");
  Serial.println(roomStatus);
  Serial.print("Ventilation: ");
  Serial.println(ventStatus);

  // Only upload every 20 seconds to stay within ThingSpeak's rate limit
  if (millis() - lastUpload >= UPLOAD_INTERVAL) {
    sendToThingSpeak(temperature, humidity, lightLevel, ledOn, ventAngle, hotHumidWarning);
    lastUpload = millis();
  }

  Serial.println("--------------------");

  delay(2000);  // check the sensors every 2 seconds
}