#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

void sendStatus(String status);

// WiFi info
const char* ssid = "N&N";
const char* password = "60209767";

// Backend URL
const char* serverUrl = "https://amac.onrender.com/api/drawers/update";

// Ultrasonic pins
const int trigPin = 5;
const int echoPin = 18;

// Measurement config
const long baseCaseDuration = 521;  // "Tom skuffe"
const long tolerance = 5;
const int numberOfSamples = 100;
const unsigned long pulseTimeout = 10000;
const int sampleDelay = 20;
const int cycleDelay = 500;

const long emptyLowerBound = baseCaseDuration - tolerance;
const long emptyUpperBound = baseCaseDuration + tolerance;

long averageDuration;
String lastStatus = "";
unsigned long lastSent = 0;
const unsigned long sendInterval = 1000;

// Reconnect WiFi if needed
void reconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("🔄 WiFi forbindelse tabt. Forsøger genforbindelse...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    Serial.println(WiFi.status() == WL_CONNECTED ? "\n✅ WiFi genforbundet!" : "\n❌ WiFi fejl.");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW);

  WiFi.begin(ssid, password);
  Serial.print("🔌 Forbinder til WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi forbundet!");
  Serial.print("📡 IP: ");
  Serial.println(WiFi.localIP());

  Serial.println("🛠️ Setup complete. Starter målinger...");
  Serial.print("📏 Tom skuffe varighed: ");
  Serial.println(baseCaseDuration);
  Serial.print("🎯 Tolerance: +/- ");
  Serial.println(tolerance);
  Serial.print("📐 Range: [");
  Serial.print(emptyLowerBound);
  Serial.print(", ");
  Serial.print(emptyUpperBound);
  Serial.println("]");
  Serial.println("──────────────────────────────");
}

void loop() {
  long totalDuration = 0;
  int validSamples = 0;

  for (int i = 0; i < numberOfSamples; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, pulseTimeout);
    if (duration > 0) {
      totalDuration += duration;
      validSamples++;
    }
    delay(sampleDelay);
  }

  if (validSamples > 0) {
    averageDuration = totalDuration / validSamples;
    String status = (averageDuration >= emptyLowerBound && averageDuration <= emptyUpperBound)
                      ? "empty"
                      : "item_detected";

    Serial.print("📏 Gennemsnit: ");
    Serial.print(averageDuration);
    Serial.print(" us → ");
    Serial.println(status);

    if (status != lastStatus && millis() - lastSent > sendInterval) {
      reconnectWiFi();  // <–– forsøg automatisk genforbindelse
      sendStatus(status);
      lastStatus = status;
      lastSent = millis();
    }
  } else {
    Serial.println("⚠️ Ingen gyldige målinger.");
  }

  delay(cycleDelay);
}

void sendStatus(String status) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<256> doc;
    doc["drawer_id"] = "drawer_ultra_01";
    doc["sensor_type"] = "ultrasound";
    doc["status"] = status;
    doc["conductive"] = false;

    String json;
    serializeJson(doc, json);

    int httpCode = http.POST(json);
    Serial.print("🌐 HTTP POST: ");
    Serial.println(httpCode);

    http.end();
  } else {
    Serial.println("🚫 WiFi ikke forbundet under sendStatus!");
  }
}
