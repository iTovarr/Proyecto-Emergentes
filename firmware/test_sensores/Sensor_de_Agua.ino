#include <WiFi.h>
#include <PubSubClient.h>
#include "Password.ino"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

const char* mqtt_server = "161.132.38.123";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

int waterPin = 4;   // D2 del ESP32
int level = 0;

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
    if (client.connect("ESP32Agua")) {
      client.subscribe("Sistema/#");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(waterPin, INPUT);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  delay(1000);
  level = digitalRead(waterPin);  // lectura digital

  Serial.print("Nivel de agua (digital): ");
  Serial.println(level);

  // Crear mensaje JSON
  String mensaje = "{";
  mensaje += "\"valor_agua\":" + String(level);
  mensaje += "}";

  client.publish("Sensores/Sensor_de_Agua", mensaje.c_str());
  Serial.println("📤 Enviado al broker MQTT:");
  Serial.println(mensaje);
}
