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
const unsigned long WIFI_RETRY_INTERVAL = 10000;

unsigned long lastUpload = 0;
unsigned long lastWiFiAttempt = 0;

DHT dht(DHTPIN, DHTTYPE);
Servo ventServo;
bool ventOpen = false;

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.println("Attempting Wi-Fi connection...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lastWiFiAttempt = millis();
}

void sendToThingSpeak(float temperature, float humidity, int lightLevel, bool ledOn, bool ventOpen) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("ThingSpeak upload skipped: Wi-Fi disconnected");
    return;
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

  // Periodically attempt to reconnect to wifi without stopping automation
  if (WiFi.status() != WL_CONNECTED &&
      millis() - lastWiFiAttempt >= WIFI_RETRY_INTERVAL) {
    connectWiFi();
  }

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  bool dhtValid = !isnan(temperature) && !isnan(humidity);

  // DHT22 failsafe
  if (!dhtValid) {
    Serial.println("Failed to read DHT22!");
    Serial.println("Temperature automation disabled");
    Serial.println("SAFE STATE: Ventilation OPEN");

    ventServo.write(90);
    ventOpen = true;
  }

  int lightLevel = analogRead(LDR_PIN);

  bool ldrValid = lightLevel > 0 && lightLevel < 4095;

  // LDR failsafe
  if (!ldrValid) {
    Serial.println("Failed to read LDR!");
    Serial.println("Automatic light control disabled");
    Serial.println("SAFE STATE: LED OFF");

    digitalWrite(LED_PIN, LOW);
  }

  if (dhtValid) {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  }

  bool ledOn = false;

  if (ldrValid) {
    Serial.print("Light Level: ");
    Serial.println(lightLevel);

    ledOn = lightLevel < LIGHT_THRESHOLD;
    digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
    Serial.println(ledOn ? "Light: ON" : "Light: OFF");
  }

  if (dhtValid) {
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
  }

  if (millis() - lastUpload >= UPLOAD_INTERVAL) {
    if (dhtValid && ldrValid) {
      sendToThingSpeak(temperature, humidity, lightLevel, ledOn, ventOpen);
    }

    lastUpload = millis();
  }

  Serial.println("--------------------");

  delay(2000);
}