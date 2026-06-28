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

// Pin del sensor de humo (DO conectado a D2 = GPIO2)
int smokePin = 2;
int estadoHumo = 0;

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
    if (client.connect("ESP32Humo")) {
      Serial.println("✅ Conectado al broker MQTT");
      // No necesitamos suscripciones aún
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
  pinMode(smokePin, INPUT);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  delay(1000);
  estadoHumo = digitalRead(smokePin);   // 1 = humo detectado, 0 = sin humo

  Serial.print("Estado de humo: ");
  Serial.println(estadoHumo);

  // Crear mensaje JSON
  String mensaje = "{";
  mensaje += "\"valor_humo\":" + String(estadoHumo);
  mensaje += "}";

  // Publicar en MQTT
  client.publish("Sensores/Sensor_de_Humo", mensaje.c_str());
  Serial.println("📤 Enviado al broker MQTT:");
  Serial.println(mensaje);
}