#include <WiFiS3.h>
#include <PubSubClient.h>
//#include "arduino_secrets.h"

// --- WiFi ---
char ssid[] = "TOVAR";
char pass[] = "29719073";

// --- MQTT ---
const char* mqtt_server = "161.132.38.123";
const int mqtt_port = 1883;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// --- Pines de luces ---
const int salaPin    = 8;
const int garajePin  = 9;
const int cocinaPin  = 10;
const int comedorPin = 11;
const int cuartoPin  = 12;

// --- Buzzer ---
const int buzzerPin = 13;

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
  if (String(topic) == "Luces/Sala")          digitalWrite(salaPin,    mensaje == "luz prendida");
  if (String(topic) == "Luces/Garaje")        digitalWrite(garajePin,  mensaje == "luz prendida");
  if (String(topic) == "Luces/Cocina")        digitalWrite(cocinaPin,  mensaje == "luz prendida");
  if (String(topic) == "Luces/Sala_Comedor")  digitalWrite(comedorPin, mensaje == "luz prendida");
  if (String(topic) == "Luces/Cuarto")        digitalWrite(cuartoPin,  mensaje == "luz prendida");

  // Control del buzzer (alarma)
  if (String(topic) == "Sistema/Alarma" && mensaje == "Alarma_Activada") {
    digitalWrite(buzzerPin, HIGH);
    delay(3000);              // mantener encendido 5 segundos
    digitalWrite(buzzerPin, LOW);
  }
}

// --- Reconexión MQTT ---
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ArduinoR4LucesAlarma")) {
      Serial.println("✅ Conectado al broker MQTT");
      client.subscribe("Luces/Sala");
      client.subscribe("Luces/Garaje");
      client.subscribe("Luces/Cocina");
      client.subscribe("Luces/Sala_Comedor");
      client.subscribe("Luces/Cuarto");
      client.subscribe("Sistema/Alarma");
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
  pinMode(buzzerPin, OUTPUT);

  digitalWrite(salaPin, LOW);
  digitalWrite(garajePin, LOW);
  digitalWrite(cocinaPin, LOW);
  digitalWrite(comedorPin, LOW);
  digitalWrite(cuartoPin, LOW);
  digitalWrite(buzzerPin, LOW);

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