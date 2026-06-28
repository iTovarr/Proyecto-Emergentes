#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "Password.ino"

// Variables WiFi
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

// Configuración MQTT
const char* mqtt_server = "161.132.38.123";
const int mqtt_port = 1883;

// Cliente WiFi y MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Dirección I2C del Arduino Uno
const int SLAVE_ADDR = 0x08;

// --- Funciones ---
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("📩 Mensaje recibido en tópico: ");
  Serial.println(topic);
}

void setup_wifi() {
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Conectado a WiFi");
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      client.subscribe("Sensores/#");
      client.subscribe("Sistema/#");
    } else {
      delay(5000);
    }
  }
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  Wire.begin(); // Maestro I2C
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

// --- Loop ---
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  Serial.println("📡 Solicitando datos al Arduino Uno...");
  Wire.requestFrom(SLAVE_ADDR, 32);
  String data = "";
  while (Wire.available()) {
    char c = Wire.read();
    data += c;
  }

  if (data.length() > 0) {
    Serial.print("📡 Datos recibidos: ");
    Serial.println(data);
    client.publish("Sensores/ArduinoUno", data.c_str());
  } else {
    Serial.println("⚠️ No se recibieron datos");
  }

  delay(2000);
}