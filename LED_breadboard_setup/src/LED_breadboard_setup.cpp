#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char *ssid = "sensors";
const char *password = "vFBaDdtH";

// Backend endpoints
const char *serverUrl = "https://amac.onrender.com/api/drawers";
const char *logUrl = "https://amac.onrender.com/api/log";

// LED pin mapping
const int redLEDs[4] = {23, 12, 17, 32};   // Empty
const int greenLEDs[4] = {22, 14, 16, 33}; // Item detected
const String drawerIds[4] = {"001", "002", "003", "004"};

// Send log to server
void sendLog(String logMessage)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(logUrl);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<256> doc;
    doc["log"] = logMessage;

    String json;
    serializeJson(doc, json);

    int httpCode = http.POST(json);
    http.end();
    Serial.println("📝 Log sent: " + logMessage);
  }
  else
  {
    Serial.println("🚫 WiFi not connected for logging");
  }
}

void setup()
{
  Serial.begin(115200);

  for (int i = 0; i < 4; i++)
  {
    pinMode(redLEDs[i], OUTPUT);
    pinMode(greenLEDs[i], OUTPUT);
    digitalWrite(redLEDs[i], LOW);
    digitalWrite(greenLEDs[i], LOW);
  }

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Connected to WiFi");
  sendLog("✅ LED unit connected to WiFi");
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(serverUrl);
    int httpCode = http.GET();

    if (httpCode == 200)
    {
      String payload = http.getString();
      Serial.println("📦 Data received:");
      Serial.println(payload);
      sendLog("📦 Data received");

      StaticJsonDocument<1024> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error)
      {
        for (int i = 0; i < 4; i++)
        {
          String status = "unknown";

          for (JsonObject drawer : doc.as<JsonArray>())
          {
            if (drawer["drawer_id"] == drawerIds[i])
            {
              status = drawer["status"].as<String>();
              Serial.println("Drawer " + drawerIds[i] + " → " + status);
              sendLog("Drawer " + drawerIds[i] + " → " + status);
              break;
            }
          }

          if (status == "empty")
          {
            digitalWrite(redLEDs[i], HIGH);
            digitalWrite(greenLEDs[i], LOW);
          }
          else if (status == "item_detected")
          {
            digitalWrite(redLEDs[i], LOW);
            digitalWrite(greenLEDs[i], HIGH);
          }
          else
          {
            digitalWrite(redLEDs[i], LOW);
            digitalWrite(greenLEDs[i], LOW);
          }
        }
      }
      else
      {
        Serial.println("❌ JSON parsing failed");
        sendLog("❌ JSON parsing failed");
      }
    }
    else
    {
      String errMsg = "❌ Failed to fetch data. HTTP code: " + String(httpCode);
      Serial.println(errMsg);
      sendLog(errMsg);
    }

    http.end();
  }
  else
  {
    Serial.println("🚫 Not connected to WiFi");
    sendLog("🚫 Not connected to WiFi");
  }

  delay(5000); // Wait 5 seconds before next check
}
