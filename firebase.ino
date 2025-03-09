#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <time.h>

#define DHTPIN 4
#define DHTTYPE DHT11
#define LDRPIN 34
#define THRESHOLD 1000 

const char* ntpServer = "time.google.com";
const long gmtOffset_sec = 8 * 3600;
const int daylightOffset_sec = 0;

const char* ssid = "Manji";
const char* password = "12345678";
const char* firestoreUrl = "https://firestore.googleapis.com/v1/projects/fir-add02/databases/(default)/documents/sensorData?key=AIzaSyCJ-OoMjfBPuVYnKHQY0RNVnjvZH3etuQw";

DHT dht(DHTPIN, DHTTYPE);
int nightCount = 0;
bool isNight = false;

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nConnected to WiFi!");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    dht.begin();
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
            Serial.println("Failed to obtain time.");
            return;
        }
        char timestamp[30];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", &timeinfo);

        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();
        int ldrValue = analogRead(LDRPIN);
        Serial.print("LDR Value: "); Serial.println(ldrValue);
        
        if (ldrValue < THRESHOLD) {
            if (!isNight) {
                nightCount++;
                isNight = true;
            }
        } else {
            isNight = false;
        }

        HTTPClient http;
        http.begin(firestoreUrl);
        http.addHeader("Content-Type", "application/json");

        String jsonData = String("{\"fields\": {") +
            "\"temperature\": {\"stringValue\": \"" + String(temperature) + "\"}," +
            "\"humidity\": {\"stringValue\": \"" + String(humidity) + "\"}," +
            "\"daylight\": {\"stringValue\": \"" + String(nightCount) + "\"}," +
            "\"timestamp\": {\"stringValue\": \"" + String(timestamp) + "\"}" +
            "}}";

        int httpResponseCode = http.POST(jsonData);
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);

        http.end();
    }
    delay(5000);
}
