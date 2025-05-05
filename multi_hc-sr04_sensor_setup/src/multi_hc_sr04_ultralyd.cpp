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
const int trigPins[NUM_SENSORS] = {21, 18, 25, 2};
const int echoPins[NUM_SENSORS] = {22, 17, 27, 4};

// --- Sensor Config ---
const long baseCaseDurations[NUM_SENSORS] = {568, 521, 538, 574}; // Kalibrer disse!
const long tolerances[NUM_SENSORS] = {8, 8, 8, 8};

const String drawerId[NUM_SENSORS] = {"001", "002", "003", "004"};
const String itemName[NUM_SENSORS] = {"uMatch 14 POS FEM", "No tag 1", "Reg 3.3V", "MOSFET"};
const String sr_code[NUM_SENSORS] = {"SR001", "SR002", "SR003", "SR004"};

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
    http.begin("https://amac.onrender.com/api/log");
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
    doc["drawer_id"] = drawerId[sensorIndex];
    doc["item_name"] = itemName[sensorIndex];
    doc["sr_code"] = sr_code[sensorIndex];
    doc["status"] = status;

    String json;
    serializeJson(doc, json);

    int httpCode = http.POST(json);
    String msg = "Sensor " + drawerId[sensorIndex] + " HTTP POST: " + String(httpCode);
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

void loop() {
  for (int i = 0; i < NUM_SENSORS; i++) {
    // ——— 1) Tag 50 prøver og beregn gennemsnit ———
    long totalDuration = 0;
    int validSamples = 0;
    for (int s = 0; s < 50; s++) {
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
    if (validSamples == 0) continue;  // intet ekko → spring over

    // ——— 2) Beregn status og debug-print med index i ———
    long avgDuration = totalDuration / validSamples;
    String currentStatus = (avgDuration >= emptyLowerBounds[i] && avgDuration <= emptyUpperBounds[i])
                               ? "empty"
                               : "item_detected";
    String debugMsg = "DEBUG → i=" + String(i)
                      + " drawer=" + drawerId[i]
                      + " avg=" + String(avgDuration)
                      + " us → " + currentStatus;
    Serial.println(debugMsg);
    sendLog(debugMsg);

    // ——— 3) Kun hvis status er ændret og der er gået > sendInterval ———
    if (currentStatus != lastStatus[i]) {
      reconnectWiFi();
      sendStatus(i, currentStatus);   // sender status for skuffe i
      lastStatus[i] = currentStatus;  // opdaterer kendt status
      lastSent[i]   = millis();       // (valgfrit) kan bruges til rate-limiting
    }
    
    // Kort pause før næste sensor
    delay(50);
  }

  // 5) Pause når alle fire er behandlet
  delay(500);
}

