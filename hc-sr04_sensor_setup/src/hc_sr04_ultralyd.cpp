#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi info
const char* ssid = "sensors";
const char* password = "vFBaDdtH";

// Backend URL
const char* serverUrl = "https://amac.onrender.com/api/drawers/update";

// --- Sensor Pins ---
const int NUM_SENSORS = 4;
const int trigPins[NUM_SENSORS] = {2, 14, 17, 18};
const int echoPins[NUM_SENSORS] = {15, 12, 16, 19};

// --- Sensor Config ---
const long baseCaseDurations[NUM_SENSORS] = {521, 530, 515, 545};  // Kalibrer disse!
const long tolerances[NUM_SENSORS] = {25, 25, 25, 25};

long emptyLowerBounds[NUM_SENSORS];
long emptyUpperBounds[NUM_SENSORS];

String lastStatus[NUM_SENSORS] = {"", "", "", ""};
unsigned long lastSent[NUM_SENSORS] = {0, 0, 0, 0};
const unsigned long sendInterval = 1000;

// --- WiFi reconnect ---
void reconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("🔄 WiFi genforbindelse...");
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

// --- Send status for each sensor ---
void sendStatus(int sensorIndex, String status) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<256> doc;
    String drawerId = "drawer_ultra_s" + String(sensorIndex + 1);
    doc["drawer_id"] = drawerId;
    doc["sensor_type"] = "ultrasound";
    doc["status"] = status;
    doc["conductive"] = false;

    String json;
    serializeJson(doc, json);

    int httpCode = http.POST(json);
    Serial.print("🌐 Sensor "); Serial.print(sensorIndex + 1); Serial.print(" HTTP POST: ");
    Serial.println(httpCode);

    http.end();
  } else {
    Serial.println("🚫 WiFi ikke forbundet under sendStatus!");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("🔌 Forbinder til WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi forbundet!");

  for (int i = 0; i < NUM_SENSORS; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
    emptyLowerBounds[i] = baseCaseDurations[i] - tolerances[i];
    emptyUpperBounds[i] = baseCaseDurations[i] + tolerances[i];
  }
  Serial.println("🛠️ Multi-sensor setup complete.");
}

void loop() {
  for (int i = 0; i < NUM_SENSORS; i++) {
    long totalDuration = 0;
    int validSamples = 0;

    for (int s = 0; s < 50; s++) {  // 50 samples pr sensor
      digitalWrite(trigPins[i], LOW);
      delayMicroseconds(2);
      digitalWrite(trigPins[i], HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPins[i], LOW);

      long duration = pulseIn(echoPins[i], HIGH, 10000);
      if (duration > 0) {
        totalDuration += duration;
        validSamples++;
      }
      delay(10);
    }

    if (validSamples > 0) {
      long avgDuration = totalDuration / validSamples;
      String status = (avgDuration >= emptyLowerBounds[i] && avgDuration <= emptyUpperBounds[i])
                      ? "empty"
                      : "item_detected";

      Serial.print("📏 Sensor "); Serial.print(i + 1); Serial.print(": ");
      Serial.print(avgDuration); Serial.print(" us → "); Serial.println(status);

      if (status != lastStatus[i] && millis() - lastSent[i] > sendInterval) {
        reconnectWiFi();
        sendStatus(i, status);
        lastStatus[i] = status;
        lastSent[i] = millis();
      }
    }
  }

  delay(500);  // Delay mellem sensorrunder
}
