#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ----------------- Configuration Section ----------------- //
// WiFi info
const char *ssid = "sensors";
const char *password = "vFBaDdtH";

// Backend URL
const char *serverUrl = "https://amac.onrender.com/api/drawers/update";

// Sensor Settings
const int trigPin = 25;
const int echoPin = 27;

const long baseCaseDuration = 538; // baseline measurement when empty
const long tolerance = 5;          // acceptable variation
const String drawerId = "005";     // unique ID for this drawer
const String itemName = "REG 3.3V";

const unsigned long sendInterval = 1000; // time between status updates
// ----------------------------------------------------------- //

long emptyLowerBound = baseCaseDuration - tolerance;
long emptyUpperBound = baseCaseDuration + tolerance;

String lastStatus = "";
unsigned long lastSent = 0;

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
    Serial.println("🔄 Reconnecting WiFi...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10)
    {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    Serial.println(WiFi.status() == WL_CONNECTED ? "\n✅ WiFi Reconnected!" : "\n❌ WiFi Failed.");
  }
}

// --- Send sensor status ---
void sendStatus(String status)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<256> doc;
    doc["drawer_id"] = drawerId;
    doc["item_name"] = itemName;
    doc["status"] = status;
    doc["sr_code"] = "xxxxxxxxx";

    String json;
    serializeJson(doc, json);

    int httpCode = http.POST(json);
    Serial.print("🌐 HTTP POST result: ");
    Serial.println(httpCode);

    http.end();
  }
  else
  {
    Serial.println("🚫 WiFi not connected during sendStatus!");
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.print("🔌 Connecting to WiFi");
  sendLog("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  sendLog("WiFi Connected");

  Serial.println("\n✅ WiFi Connected!");
}

void loop()
{
  long totalDuration = 0;
  int validSamples = 0;

  // Take 50 samples
  for (int s = 0; s < 50; s++)
  {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 10000);
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
    String status = (avgDuration >= emptyLowerBound && avgDuration <= emptyUpperBound)
                        ? "empty"
                        : "item_detected";

    String statusMsg = "Sensor " + itemName + ": " + String(avgDuration) + " us → " + status;

    // Serial.print("📏 Avg Duration: ");
    // Serial.print(avgDuration);
    // Serial.print(" us → ");
    Serial.println(statusMsg);
    sendLog(statusMsg);

    if (status != lastStatus && millis() - lastSent > sendInterval)
    {
      reconnectWiFi();
      sendStatus(status);
      lastStatus = status;
      lastSent = millis();
    }
  }

  delay(500); // Delay between measurements
}
