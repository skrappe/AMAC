#include <Arduino.h>
#include <WiFi.h>        // Required for WiFi functionality
#include <HTTPClient.h> // Required for making HTTP requests (ensure ESP32 core is installed)
#include <ArduinoJson.h> // Required for creating JSON payloads
//#include <secrets.h>

// --- WiFi Configuration ---
const char *ssid = "sensors";      // Your WiFi Network Name
const char *password = "vFBaDdtH"; // Your WiFi Password

// --- Backend Configuration ---
const char *serverUrl = "http://amac.onrender.com/api/drawers/update"; // Your Backend URL

// --- Sensor Configuration ---
const int NUM_SENSORS = 4;

// Pin Configuration (Using pins from your second file)
// Sensor 1: Echo 15, Trig 2
// Sensor 2: Echo 12, Trig 14
// Sensor 3: Echo 16, Trig 17
// Sensor 4: Echo 19, Trig 18
// ** Please double-check your actual wiring and adjust if needed. **
const int echoPins[NUM_SENSORS] = {15, 12, 16, 19};
const int trigPins[NUM_SENSORS] = {2, 14, 17, 18};

// Detection Configuration (MUST BE CALIBRATED PER SENSOR POSITION!)
// Using base cases and tolerances from your second file
const long baseCaseDurations[NUM_SENSORS] = {521, 530, 515, 545}; // Example: Calibrate per sensor
const long tolerances[NUM_SENSORS] = {25, 30, 25, 25};            // Example: Calibrate per sensor

// Calculated bounds (per sensor) - will be filled in setup()
long emptyLowerBounds[NUM_SENSORS];
long emptyUpperBounds[NUM_SENSORS];

// --- Timing & Sampling Configuration ---
const int numberOfSamples = 100;          // How many readings to average per sensor per cycle
const unsigned long pulseTimeout = 10000; // Max wait time for pulseIn (10ms)
const int sampleDelay = 20;               // ms - Delay AFTER reading all sensors in one sample pass
const int cycleDelay = 500;               // ms - Delay between full measurement cycles

// --- State & Sending Configuration ---
String lastSentStatuses[NUM_SENSORS];          // Stores the last status sent for each sensor
unsigned long lastSentTime[NUM_SENSORS] = {0}; // Stores the last time status was sent for each sensor
const unsigned long sendInterval = 1000;       // Minimum interval (ms) between sending updates for the *same* sensor

//_________________________________________________________________________
//            HELPER FUNCTIONS (WiFi Reconnect & Send Status)
//_________________________________________________________________________

// Reconnect WiFi if needed
void reconnectWiFi()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("🔄 WiFi forbindelse tabt. Forsøger genforbindelse...");
    WiFi.disconnect();   // Ensure clean disconnect before reconnecting
    WiFi.mode(WIFI_STA); // Set WiFi mode to Station explicitly
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    { // Increased attempts
      delay(500);
      Serial.print(".");
      attempts++;
    }
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("\n✅ WiFi genforbundet!");
      Serial.print("📡 IP: ");
      Serial.println(WiFi.localIP());
    }
    else
    {
      Serial.println("\n❌ WiFi genforbindelse fejlede.");
    }
  }
}

// Send status for a specific sensor
void sendStatus(int sensorIndex, String status)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(serverUrl); // Specify URL here
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<256> doc; // Create JSON document

    // --- Customize Payload ---
    // Create a unique drawer_id based on the sensor index
    String drawerId = "drawer_ultra_s" + String(sensorIndex + 1);
    doc["drawer_id"] = drawerId;
    doc["sensor_type"] = "ultrasound";
    doc["status"] = status; // "empty" or "item_detected"
    doc["conductive"] = false; // Eller true hvis relevant


    String jsonPayload;
    serializeJson(doc, jsonPayload); // Convert JSON document to String

    Serial.print("📤 Sender til backend: ");
    Serial.println(jsonPayload);

    // Send the POST request
    int httpCode = http.POST(jsonPayload);

    // Check response code
    if (httpCode > 0)
    {
      Serial.print("✅ HTTP POST succes, response code: ");
      Serial.println(httpCode);
      // String responsePayload = http.getString(); // Optional: get response body
      // Serial.println(responsePayload);
    }
    else
    {
      Serial.print("❌ HTTP POST fejlede, error: ");
      Serial.println(http.errorToString(httpCode).c_str());
    }

    http.end(); // Free resources
  }
  else
  {
    Serial.println("🚫 WiFi ikke forbundet under sendStatus!");
  }
}

//_________________________________________________________________________
//                             SETUP FUNCTION
//_________________________________________________________________________

void setup()
{
  Serial.begin(115200); // Use 115200 for ESP32 usually
  while (!Serial)
    ; // Wait for Serial port to connect (needed for some boards)

  Serial.println("\n==============================");
  Serial.println(" Multi-Sensor HC-SR04 Logger");
  Serial.println("==============================");

  // --- Initialize Sensor Pins & Calculate Bounds ---
  Serial.println("🔧 Initialiserer sensorer...");
  for (int i = 0; i < NUM_SENSORS; i++)
  {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
    digitalWrite(trigPins[i], LOW); // Ensure trigger pins start low

    emptyLowerBounds[i] = baseCaseDurations[i] - tolerances[i];
    emptyUpperBounds[i] = baseCaseDurations[i] + tolerances[i];
    lastSentStatuses[i] = ""; // Initialize last sent status as empty string

    // Print configuration details for user verification
    Serial.print("  Sensor ");
    Serial.print(i + 1);
    Serial.print(" [Trig:");
    Serial.print(trigPins[i]);
    Serial.print(", Echo:");
    Serial.print(echoPins[i]);
    Serial.print("] - Base: ");
    Serial.print(baseCaseDurations[i]);
    Serial.print(" us, Tol: +/-");
    Serial.print(tolerances[i]);
    Serial.print(" us => Range: [");
    Serial.print(emptyLowerBounds[i]);
    Serial.print(", ");
    Serial.print(emptyUpperBounds[i]);
    Serial.println("] us");
  }

  // --- Initialize WiFi ---
  Serial.println("🔌 Forbinder til WiFi...");
  WiFi.mode(WIFI_STA); // Set WiFi mode to Station
  WiFi.begin(ssid, password);
  Serial.print("   Netværk: ");
  Serial.println(ssid);
  int wifi_attempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifi_attempts < 30)
  { // Wait longer if needed
    delay(500);
    Serial.print(".");
    wifi_attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\n✅ WiFi forbundet!");
    Serial.print("   IP Adresse: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("\n❌ Kunne ikke forbinde til WiFi. Fortsætter offline.");
    // Consider error handling here - maybe restart?
  }

  Serial.println("--------------------------");
  Serial.println("🚀 Starter målinger...");
  Serial.println("--------------------------");
}

//_________________________________________________________________________
//                              MAIN LOOP
//_________________________________________________________________________

void loop()
{
  // Arrays to hold data for this measurement cycle
  long totalDurations[NUM_SENSORS] = {0};
  int validSampleCounts[NUM_SENSORS] = {0};
  long averageDurations[NUM_SENSORS] = {0};
  String currentStatuses[NUM_SENSORS]; // Holds the calculated status for this cycle

  // --- 1. Collect Samples Sequentially for all Sensors ---
  for (int i = 0; i < numberOfSamples; i++)
  {
    for (int sensorIndex = 0; sensorIndex < NUM_SENSORS; sensorIndex++)
    {
      // Generate ultrasonic pulse
      digitalWrite(trigPins[sensorIndex], LOW);
      delayMicroseconds(2);
      digitalWrite(trigPins[sensorIndex], HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPins[sensorIndex], LOW);

      // Read echo duration
      long duration = pulseIn(echoPins[sensorIndex], HIGH, pulseTimeout);

      // Store valid readings
      if (duration > 0)
      {
        totalDurations[sensorIndex] += duration;
        validSampleCounts[sensorIndex]++;
      }
      // Optional short delay between sensors if needed
      // delay(10);
    } // End sensor loop
    delay(sampleDelay); // Delay between sample sets
  } // End sample collection loop

  // --- 2. Process Results and Send Status for Each Sensor ---
  // Serial.println("--- Cyklus Resultater ---"); // Optional: less verbose output
  unsigned long currentTime = millis(); // Get current time once for efficiency

  for (int sensorIndex = 0; sensorIndex < NUM_SENSORS; sensorIndex++)
  {
    Serial.print("S");
    Serial.print(sensorIndex + 1);
    Serial.print(": "); // Compact output prefix

    if (validSampleCounts[sensorIndex] > 0)
    {
      // Calculate average duration
      averageDurations[sensorIndex] = totalDurations[sensorIndex] / validSampleCounts[sensorIndex];

      // Determine current status ("empty" or "item_detected")
      if (averageDurations[sensorIndex] >= emptyLowerBounds[sensorIndex] && averageDurations[sensorIndex] <= emptyUpperBounds[sensorIndex])
      {
        currentStatuses[sensorIndex] = "empty";
      }
      else
      {
        currentStatuses[sensorIndex] = "item_detected";
      }

      // Print results locally
      Serial.print(averageDurations[sensorIndex]);
      Serial.print("us [");
      Serial.print(validSampleCounts[sensorIndex]);
      Serial.print("] -> ");
      Serial.print(currentStatuses[sensorIndex]);

      // Check if status changed AND enough time passed since last send FOR THIS SENSOR
      if (currentStatuses[sensorIndex] != lastSentStatuses[sensorIndex])
      {
        if (currentTime - lastSentTime[sensorIndex] > sendInterval)
        {
          Serial.print(" | Status Ændret! Sender...");                  // Indicate status change and intent to send
          reconnectWiFi();                                              // Check WiFi connection before sending
          sendStatus(sensorIndex, currentStatuses[sensorIndex]);        // Send the new status
          lastSentStatuses[sensorIndex] = currentStatuses[sensorIndex]; // Update last sent status
          lastSentTime[sensorIndex] = currentTime;                      // Update last sent time
          Serial.println();                                             // Newline after sending confirmation/error
        }
        else
        {
          Serial.println(" | Status Ændret (Interval Ikke Mødt)"); // Status changed but too soon to send
        }
      }
      else
      {
        Serial.println(" | Status Stabil"); // Status hasn't changed
      }
    }
    else
    {
      // No valid readings for this sensor in this cycle
      Serial.println("Ingen gyldige målinger! (Timeout/Obstruktion?)");
      // Decide if you want to send a special status here, e.g., "error" or "unknown"
      // Example: if("error" != lastSentStatuses[sensorIndex] && currentTime - lastSentTime[sensorIndex] > sendInterval) { ... sendStatus(sensorIndex, "error"); ...}
      currentStatuses[sensorIndex] = "error"; // Store error status locally for potential future send logic
    }
  } // End processing loop for sensors

  // Serial.println("--------------------------"); // Optional: less verbose

  // Delay before starting the next full measurement cycle
  delay(cycleDelay);
}