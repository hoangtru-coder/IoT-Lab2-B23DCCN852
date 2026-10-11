#include <WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const char* MQTT_SERVER = "broker.hivemq.com";
const int MQTT_PORT = 1883;

// ĐÃ XÓA BIẾN TOKEN VÌ HIVEMQ PUBLIC KHÔNG CẦN

const int DHT_PIN = 15;
const int TRIG_PIN = 5;
const int ECHO_PIN = 18;
const int LED_PIN = 2;

const unsigned long SEND_INTERVAL_MS = 5000;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
DHTesp dht;

unsigned long lastSend = 0;
unsigned long sequenceNo = 0;

// ================= WIFI =================
void connectWiFi() {
  Serial.print("Connecting WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ================= MQTT =================
void connectMQTT() {
  while (!mqttClient.connected()) {
    String clientId = "ESP32-Bai2-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    Serial.print("Connecting MQTT...");

    // SỬA LỖI: Gọi hàm connect không cần TOKEN
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println(" connected");
    } else {
      Serial.print(" failed, rc=");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

// ================= HC-SR04 =================
float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return NAN;
  }
  return duration * 0.0343 / 2.0;
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("======================");
  Serial.println("ESP32 START");
  Serial.println("======================");

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  dht.setup(DHT_PIN, DHTesp::DHT22);

  connectWiFi();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setBufferSize(256);
}

// ================= LOOP =================
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqttClient.connected()) {
    connectMQTT();
  }

  mqttClient.loop();

  unsigned long now = millis();

  if (now - lastSend < SEND_INTERVAL_MS) {
    return;
  }

  lastSend = now;

  TempAndHumidity data = dht.getTempAndHumidity();
  float distance = readDistanceCm();

  if (!isfinite(data.temperature) ||
      !isfinite(data.humidity) ||
      !isfinite(distance)) {
    Serial.println("Invalid sensor data - skip publish");
    return;
  }

  sequenceNo++;

  char payload[256];
  snprintf(
    payload,
    sizeof(payload),
    "{\"temperature\":%.2f,"
    "\"humidity\":%.2f,"
    "\"distance_cm\":%.2f,"
    "\"rssi\":%d,"
    "\"sequence\":%lu,"
    "\"uptime_s\":%lu}",
    data.temperature,
    data.humidity,
    distance,
    WiFi.RSSI(),
    sequenceNo,
    millis() / 1000
  );

  // SỬA LỖI: Đổi topic gửi lên thành topic định danh cá nhân
  bool ok = mqttClient.publish(
      "ptit/iot/lab2/B23DCCN852/sensors",
      payload
  );

  Serial.print("DATA: ");
  Serial.println(payload);
  Serial.print("PUBLISH: ");
  Serial.println(ok ? "OK" : "FAILED");

  digitalWrite(LED_PIN, HIGH);
  delay(80);
  digitalWrite(LED_PIN, LOW);
}
