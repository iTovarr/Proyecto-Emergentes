#include <WiFi.h>
#include <PubSubClient.h>
#include "Password.ino"

// WiFi
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

// MQTT
const char* mqtt_server = "161.132.38.123";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// Pines del sensor ultrasónico
const int trigPin = 19;  // TRIG en D19
const int echoPin = 18;  // ECHO en D18

long duration;
float distance;

void setup_wifi() {
  WiFi.begin(ssid, pass);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Conectado a WiFi");
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Movimiento")) {
      Serial.println("✅ Conectado al broker MQTT");
      client.subscribe("Sistema/#"); // suscripción opcional
    } else {
      Serial.print("❌ Error, rc=");
      Serial.print(client.state());
      Serial.println(" -> Reintentando en 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Generar pulso en TRIG
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Medir duración del pulso en ECHO
  duration = pulseIn(echoPin, HIGH);

  // Calcular distancia en cm
  distance = duration * 0.034 / 2;

  Serial.print("Distancia: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Crear mensaje JSON con formato exacto
  String mensaje = "{ \"distancia\": " + String(distance, 2) + " }";

  // Publicar en MQTT
  client.publish("Sensores/Sensor_Proximidad", mensaje.c_str());
  Serial.println("📤 Enviado al broker MQTT:");
  Serial.println(mensaje);

  delay(1000);
}