#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char *ssid = "sensors";
const char *password = "vFBaDdtH";

// Backend endpoint
const char *serverUrl = "https://amac.onrender.com/api/drawers";

// LED pin mapping
const int redLEDs[4] = {23, 12, 17, 32};   // Empty
const int greenLEDs[4] = {22, 14, 16, 33}; // Item detected
// Drawer IDs
const String drawerIds[4] = {"001", "002", "003", "004"};
void setup()
{
  Serial.begin(115200);

  // Set pin modes
  for (int i = 0; i < 4; i++)
  {
    pinMode(redLEDs[i], OUTPUT);
    pinMode(greenLEDs[i], OUTPUT);
    digitalWrite(redLEDs[i], LOW);
    digitalWrite(greenLEDs[i], LOW);
  }

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Connected to WiFi");
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
              Serial.println("found drawer ID: " + drawerIds[i] + " and status is: " + status);

              break;
            }
          }

          // Update LEDs
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
            // Unknown status → turn off both LEDs
            digitalWrite(redLEDs[i], LOW);
            digitalWrite(greenLEDs[i], LOW);
          }
        }
      }
      else
      {
        Serial.println("❌ JSON parsing failed");
      }
    }
    else
    {
      Serial.println("❌ Failed to fetch data. HTTP code: " + String(httpCode));
    }

    http.end();
  }
  else
  {
    Serial.println("🚫 Not connected to WiFi");
  }

  delay(5000); // Wait 5 seconds before next check
}
