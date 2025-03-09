#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>

const char* ntpServer = "time.google.com";
const long gmtOffset_sec = 8 * 3600; // GMT+8 for Philippines
const int daylightOffset_sec = 0;

const char* ssid = "Manji";
const char* password = "12345678";
const char* firestoreUrl = "https://firestore.googleapis.com/v1/projects/fir-add02/databases/(default)/documents/sensorData?key=AIzaSyCJ-OoMjfBPuVYnKHQY0RNVnjvZH3etuQw";

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  int max_attempts = 20;
  while (WiFi.status() != WL_CONNECTED && max_attempts > 0) {
    Serial.print(".");
    delay(1000);
    max_attempts--;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi!");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  } else {
    Serial.println("\nFailed to connect to WiFi.");
    return;
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    struct tm timeinfo;
    int retry_count = 5;
    while (!getLocalTime(&timeinfo) && retry_count > 0) {
      Serial.println("Retrying time sync...");
      delay(1000);
      retry_count--;
    }
    if (retry_count == 0) {
      Serial.println("Failed to obtain time.");
      return;
    }

    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", &timeinfo);

    HTTPClient http;
    http.begin(firestoreUrl);
    http.addHeader("Content-Type", "application/json");

    String jsonData = String("{\"fields\": {") +
        "\"temperature\": {\"stringValue\": \"25.5\"}," +
        "\"humidity\": {\"stringValue\": \"60\"}," +
        "\"daylight\": {\"stringValue\": \"21\"}," +
        "\"timestamp\": {\"stringValue\": \"" + String(timestamp) + "\"}" +
        "}}";

    int httpResponseCode = http.POST(jsonData);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error: " + http.errorToString(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("WiFi disconnected. Attempting to reconnect...");
    WiFi.disconnect();
    WiFi.reconnect();
    int retries = 10;
    while (WiFi.status() != WL_CONNECTED && retries > 0) {
      delay(1000);
      retries--;
    }
  }

  delay(5000);
}
