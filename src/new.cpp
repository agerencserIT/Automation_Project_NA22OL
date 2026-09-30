#include <Arduino.h>
#include <DHT.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define LDR_PIN 34
#define LED_PIN 23
const int LIGHT_THRESHOLD = 2000;

#define DHTPIN 14
#define DHTTYPE DHT22

#define SERVO_PIN 19

// Wokwi's simulated Wi-Fi network (no password)
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

const char* THINGSPEAK_API_KEY = "T843ILUU2ACWK5AZ";
const unsigned long UPLOAD_INTERVAL = 20000;  // free tier allows 1 update per 15s
unsigned long lastUpload = 0;

DHT dht(DHTPIN, DHTTYPE);
Servo ventServo;
bool ventOpen = false;

void connectWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println(" connected");
}

void sendToThingSpeak(float temperature, float humidity, int lightLevel, bool ledOn, bool ventOpen) {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  String url = "http://api.thingspeak.com/update?api_key=" + String(THINGSPEAK_API_KEY) +
               "&field1=" + String(temperature) +
               "&field2=" + String(humidity) +
               "&field3=" + String(lightLevel) +
               "&field4=" + String(ledOn ? 1 : 0) +
               "&field5=" + String(ventOpen ? 1 : 0);

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    Serial.println("ThingSpeak: entry " + http.getString());  // "0" means rejected
  } else {
    Serial.printf("ThingSpeak error: %d\n", httpCode);
  }
  http.end();
}

void setup() {

  pinMode(LDR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);

  dht.begin();

  ventServo.attach(SERVO_PIN);
  ventServo.write(0);

  connectWiFi();

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

  bool ledOn = lightLevel < LIGHT_THRESHOLD;
  digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
  Serial.println(ledOn ? "Light: ON" : "Light: OFF");

  if (temperature > 30) {
    ventServo.write(90);
    ventOpen = true;
    Serial.println("Ventilation: OPEN");
  }
  else if (temperature < 27) {
    ventServo.write(0);
    ventOpen = false;
    Serial.println("Ventilation: CLOSED");
  }

  if (millis() - lastUpload >= UPLOAD_INTERVAL) {
    sendToThingSpeak(temperature, humidity, lightLevel, ledOn, ventOpen);
    lastUpload = millis();
  }

  Serial.println("--------------------");

  delay(2000);
}