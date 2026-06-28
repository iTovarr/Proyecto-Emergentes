#include <WiFiS3.h>
#include <PubSubClient.h>
#include "Password.ino"

// --- WiFi ---
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

// --- MQTT ---
const char* mqtt_server = "161.132.38.123";
const int mqtt_port = 1883;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// --- Pines de luces ---
const int salaPin = 2;
const int garajePin = 3;
const int cocinaPin = 4;
const int comedorPin = 5;
const int cuartoPin = 6;

// --- Conexión WiFi ---
void setup_wifi() {
  WiFi.begin(ssid, pass);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Conectado a WiFi");
}

// --- Callback MQTT ---
void callback(char* topic, byte* payload, unsigned int length) {
  String mensaje;
  for (unsigned int i = 0; i < length; i++) {
    mensaje += (char)payload[i];
  }

  Serial.print("📩 Mensaje recibido en ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(mensaje);

  // Control de luces según tópico y mensaje
  if (String(topic) == "Luces/Sala") digitalWrite(salaPin, mensaje == "luz prendida");
  if (String(topic) == "Luces/Garaje") digitalWrite(garajePin, mensaje == "luz prendida");
  if (String(topic) == "Luces/Cocina") digitalWrite(cocinaPin, mensaje == "luz prendida");
  if (String(topic) == "Luces/Sala_Comedor") digitalWrite(comedorPin, mensaje == "luz prendida");
  if (String(topic) == "Luces/Cuarto") digitalWrite(cuartoPin, mensaje == "luz prendida");
}

// --- Reconexión MQTT ---
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ArduinoR4Luces")) {
      Serial.println("✅ Conectado al broker MQTT");
      client.subscribe("Luces/Sala");
      client.subscribe("Luces/Garaje");
      client.subscribe("Luces/Cocina");
      client.subscribe("Luces/Sala_Comedor");
      client.subscribe("Luces/Cuarto");
    } else {
      Serial.print("❌ Error, rc=");
      Serial.print(client.state());
      Serial.println(" -> Reintentando en 5 segundos");
      delay(5000);
    }
  }
}

// --- Setup ---
void setup() {
  Serial.begin(115200);

  pinMode(salaPin, OUTPUT);
  pinMode(garajePin, OUTPUT);
  pinMode(cocinaPin, OUTPUT);
  pinMode(comedorPin, OUTPUT);
  pinMode(cuartoPin, OUTPUT);

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
}
