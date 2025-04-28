#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi info
const char *ssid = "sensors";
const char *password = "vFBaDdtH";

// Backend URL
const char *serverUrl = "https://amac.onrender.com/api/drawers/update";

// --- Sensor Pins ---
const int NUM_SENSORS = 4;
const int trigPins[NUM_SENSORS] = {2, 25, 17, 18};
const int echoPins[NUM_SENSORS] = {15, 27, 16, 19};

// --- Sensor Config ---
const long baseCaseDurations[NUM_SENSORS] = {521, 551, 515, 545}; // Kalibrer disse!
const long tolerances[NUM_SENSORS] = {5, 5, 5, 5};

long emptyLowerBounds[NUM_SENSORS];
long emptyUpperBounds[NUM_SENSORS];
String lastStatus[NUM_SENSORS] = {"", "", "", ""};
unsigned long lastSent[NUM_SENSORS] = {0, 0, 0, 0};
const unsigned long sendInterval = 1000;

// Send log to server
void sendLog(String logMessage)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin("https://amac.onrender.com/");
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<256> doc;
    doc["log"] = logMessage;

    String json;
    serializeJson(doc, json);

    int httpCode = http.POST(json);
    http.end();
  }
  else
  {
    Serial.println("🚫 WiFi ikke forbundet under logsend!");
  }
}

// --- WiFi reconnect ---
void reconnectWiFi()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    String msg = "🔄 WiFi genforbindelse...";
    Serial.println(msg);
    sendLog(msg);
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10)
    {
      delay(500);
      String msg = ".";
      Serial.print(msg);
      sendLog(msg);
      attempts++;
    }
    msg = WiFi.status() == WL_CONNECTED ? "\n✅ WiFi genforbundet!" : "\n❌ WiFi fejl.";
    Serial.println(msg);
    sendLog(msg);
  }
}

// --- Send status for each sensor ---
void sendStatus(int sensorIndex, String status)
{
  if (WiFi.status() == WL_CONNECTED)
  {
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
    String msg = "🌐 Sensor " + String(sensorIndex + 1) + " HTTP POST: " + String(httpCode);
    Serial.println(msg);
    sendLog(msg);

    http.end();
  }
  else
  {
    String msg = "🚫 WiFi ikke forbundet under sendStatus!";
    Serial.println(msg);
    sendLog(msg);
  }
}

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  String connecting = "Forbinder til WiFi";
  Serial.println(connecting);
  // sendLog(connecting);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    String dot = ".";
    Serial.print(dot);
    sendLog(dot);
  }
  String connectedMsg = "\n✅ WiFi forbundet!";
  Serial.println(connectedMsg);
  sendLog(connectedMsg);

  for (int i = 0; i < NUM_SENSORS; i++)
  {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
    emptyLowerBounds[i] = baseCaseDurations[i] - tolerances[i];
    emptyUpperBounds[i] = baseCaseDurations[i] + tolerances[i];
  }
  String setupMsg = "🛠️ Multi-sensor setup complete.";
  Serial.println(setupMsg);
  sendLog(setupMsg);
}

void loop()
{
  for (int i = 0; i < NUM_SENSORS; i++)
  {
    long totalDuration = 0;
    int validSamples = 0;

    for (int s = 0; s < 50; s++)
    { // 50 samples pr sensor
      digitalWrite(trigPins[i], LOW);
      delayMicroseconds(2);
      digitalWrite(trigPins[i], HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPins[i], LOW);

      long duration = pulseIn(echoPins[i], HIGH, 10000);
      if (duration > 0)
      {
        totalDuration += duration;
        validSamples++;
      }
      delay(10);
    }

    if (validSamples > 0)
    {
      long avgDuration = totalDuration / validSamples;
      String status = (avgDuration >= emptyLowerBounds[i] && avgDuration <= emptyUpperBounds[i])
                          ? "empty"
                          : "item_detected";

      String statusMsg = "Sensor " + String(i + 1) + ": " + String(avgDuration) + " us → " + status;
      Serial.println(statusMsg);
      sendLog(statusMsg);

      String logMessage = "Measurement: " + String(avgDuration) + " us, Status: " + status;
      sendLog(logMessage);

      if (status != lastStatus[i] && millis() - lastSent[i] > sendInterval)
      {
        reconnectWiFi();
        sendStatus(i, status);
        lastStatus[i] = status;
        lastSent[i] = millis();
      }
    }
  }

  delay(500); // Delay mellem sensorrunder
}
