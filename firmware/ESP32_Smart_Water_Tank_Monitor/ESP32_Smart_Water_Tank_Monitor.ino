
#include <WiFi.h>
#include <PubSubClient.h>
#include "secrets.h"

// WiFi credentials
// MQTT broker
// MQTT topic
// HC-SR04 pins
#define TRIG_PIN 5
#define ECHO_PIN 18

// Bottle dimensions
const float TANK_HEIGHT = 27.0;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

SemaphoreHandle_t dataMutex;

float distance = 0;
float waterLevel = 0;
float fillPercent = 0;

void connectWiFiTask(void *parameter) {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("WiFi status: ");
Serial.println(WiFi.status());
  }

  Serial.println("WiFi Connected!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  vTaskDelete(NULL);
}

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return -1;
  }

  return duration * 0.0343 / 2;
}

void sensorTask(void *parameter) {
  while (true) {
    float measuredDistance = getDistance();

    if (measuredDistance >= 0) {
      float calculatedWaterLevel = TANK_HEIGHT - measuredDistance;

      if (calculatedWaterLevel < 0) {
        calculatedWaterLevel = 0;
      }

      if (calculatedWaterLevel > TANK_HEIGHT) {
        calculatedWaterLevel = TANK_HEIGHT;
      }

      float calculatedFillPercent =
        (calculatedWaterLevel / TANK_HEIGHT) * 100.0;

      xSemaphoreTake(dataMutex, portMAX_DELAY);

      distance = measuredDistance;
      waterLevel = calculatedWaterLevel;
      fillPercent = calculatedFillPercent;

      xSemaphoreGive(dataMutex);

      Serial.print("Distance: ");
      Serial.print(measuredDistance);
      Serial.print(" cm | Water Level: ");
      Serial.print(calculatedWaterLevel);
      Serial.print(" cm | Fill: ");
      Serial.print(calculatedFillPercent);
      Serial.println("%");
    } else {
      Serial.println("Sensor: No echo received!");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void mqttTask(void *parameter) {
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(pdMS_TO_TICKS(500));
  }

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);

  while (true) {
    if (WiFi.status() != WL_CONNECTED) {
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }

    if (!mqttClient.connected()) {
      Serial.println("Connecting to MQTT...");

      if (mqttClient.connect("ESP32_WaterTank")) {
        Serial.println("MQTT Connected!");
      } else {
        Serial.print("MQTT connection failed. State: ");
        Serial.println(mqttClient.state());

        vTaskDelay(pdMS_TO_TICKS(2000));
        continue;
      }
    }

    mqttClient.loop();

    float currentDistance;
    float currentWaterLevel;
    float currentFillPercent;

    xSemaphoreTake(dataMutex, portMAX_DELAY);

    currentDistance = distance;
    currentWaterLevel = waterLevel;
    currentFillPercent = fillPercent;

    xSemaphoreGive(dataMutex);

    char message[100];

    snprintf(
      message,
      sizeof(message),
      "{\"distance\":%.2f,\"waterLevel\":%.2f,\"fillPercent\":%.2f}",
      currentDistance,
      currentWaterLevel,
      currentFillPercent
    );

    if (mqttClient.publish(MQTT_TOPIC, message)) {
      Serial.print("Published: ");
      Serial.println(message);
    } else {
      Serial.println("MQTT publish failed!");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  dataMutex = xSemaphoreCreateMutex();

  xTaskCreate(
    connectWiFiTask,
    "WiFi Task",
    4096,
    NULL,
    1,
    NULL
  );

  xTaskCreate(
    sensorTask,
    "Sensor Task",
    4096,
    NULL,
    1,
    NULL
  );

  xTaskCreate(
    mqttTask,
    "MQTT Task",
    4096,
    NULL,
    1,
    NULL
  );
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
