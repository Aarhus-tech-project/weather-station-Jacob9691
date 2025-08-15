#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFiS3.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <ArduinoJson.h>

// default pressure until fetched
float seaLevelPressure = 1013.25;
unsigned long lastPressureUpdate = 0;
const unsigned long pressureUpdateInterval = 24UL * 60UL * 60UL * 1000UL; // 24 hours

// WiFi credentials
const char* ssid = "h4prog";
const char* password = "1234567890";

// MQTT broker settings
const char* mqttBroker = "192.168.103.11";
int mqttPort = 1883;
const char* mqttTopic = "weather/station1";

// Sensor
Adafruit_BME280 bme;  

// WiFi and MQTT clients
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// NTP client
WiFiUDP ntpUDP;
// Offset 0 = UTC. 7200sec = UTC+2
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7200, 60000); 

void wifiConnection() {
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    delay(2000);
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi");
  }
}

void MQTTConnection() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection... ");
    String clientId = "arduinoClient-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(mqttTopic);
    } else {
      Serial.print("Failed. Error code = ");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void checkConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected. Reconnecting...");
    wifiConnection();
  }

  if (!client.connected()) {
    Serial.println("MQTT disconnected.");
    MQTTConnection();
  }

  client.loop(); 
}

float fetchSeaLevelPressure() {
  const char* host = "api.open-meteo.com";
  String url = "/v1/forecast?latitude=56.113649034605984&longitude=10.12641084801739&current=surface_pressure";

  if (!wifiClient.connect(host, 80)) {
    Serial.println("Connection failed to Open-Meteo");
    return seaLevelPressure; // fallback
  }

  wifiClient.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Connection: close\r\n\r\n");

  // Skip HTTP headers
  while (wifiClient.connected()) {
    String line = wifiClient.readStringUntil('\n');
    if (line == "\r") break;
  }

  // Read JSON body
  String json = wifiClient.readString();
  wifiClient.stop();

  // Debug
  Serial.println("Raw JSON:");
  Serial.println(json);

  // Parse
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.c_str());
    return seaLevelPressure;
  }

  float pressure = doc["current"]["surface_pressure"].as<float>();
  if (pressure > 800 && pressure < 1100) {
    Serial.print("Fetched sea level pressure: ");
    Serial.println(pressure);
    return pressure;
  }

  return seaLevelPressure;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  wifiConnection();

  client.setServer(mqttBroker, mqttPort);
  MQTTConnection();

  // Start NTP
  timeClient.begin();
  timeClient.update(); // Initial time sync
}

void loop() {
  while (WiFi.status() != WL_CONNECTED || !client.connected()) {
    checkConnection();
  }

  if (WiFi.status() == WL_CONNECTED && client.connected()) {
    // Update NTP time
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    setTime(epochTime);  // Set time for TimeLib

    char timestampNumber[15]; // 14 digits + null terminator
    sprintf(timestampNumber, "%04d%02d%02d%02d%02d%02d", 
      year(), month(), day(), hour(), minute(), second());

    // Update sea-level pressure once per day
    if (millis() - lastPressureUpdate > pressureUpdateInterval) {
      lastPressureUpdate = millis();
      seaLevelPressure = fetchSeaLevelPressure();
    }

    // Read sensor data
    float temp = bme.readTemperature();
    float pressure = bme.readPressure() / 100.0F;
    float altitude = bme.readAltitude(seaLevelPressure);
    float humidity = bme.readHumidity();

    // Create JSON payload
    String payload = "{";
    payload += "\"timestamp\":" + String(timestampNumber) + ",";
    payload += "\"temperature\":" + String(temp, 2) + ",";
    payload += "\"pressure\":" + String(pressure, 2) + ",";
    payload += "\"altitude\":" + String(altitude, 2) + ",";
    payload += "\"humidity\":" + String(humidity, 2);
    payload += "}";

    // Publish
    Serial.print("Publishing to MQTT: ");
    Serial.println(payload);
    bool success = client.publish(mqttTopic, payload.c_str());
    if (success) {
      Serial.println("Data sent.");
      Serial.println("Waiting for 1 min...\n");
      delay(60000); // 1 minute
    } else {
      Serial.println("Failed to send data.\n");
      delay(5000);  
    }
  } else {
    Serial.println("Skipping publish: Not connected to MQTT or WiFi");
    delay(5000);
  }
}