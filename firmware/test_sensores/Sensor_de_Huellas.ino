#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
#include "Password.ino"
#include "time.h"

// Variables WiFi
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

// Configuración MQTT
const char* mqtt_server = "161.132.38.123";
const int mqtt_port = 1883;

// Cliente WiFi y MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Sensor de huellas en Serial2
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Configuración NTP
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -5 * 3600; // Perú GMT-5
const int daylightOffset_sec = 0;

// --- Funciones ---
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
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP32Huella")) {
      Serial.println("✅ Conectado al broker");
      client.subscribe("Sistema/#");
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

  // Inicializar huellas
  mySerial.begin(57600, SERIAL_8N1, 16, 17); // RX=16, TX=17
  finger.begin(57600);
  delay(5);

  if (finger.verifyPassword()) {
    Serial.println("✅ Sensor de huella encontrado!");
  } else {
    Serial.println("❌ No se detecta el sensor");
    while (1) { delay(1); }
  }

  finger.getTemplateCount();
  Serial.print("📊 Huellas registradas: ");
  Serial.println(finger.templateCount);

  // Conexión WiFi y MQTT
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

  // Configurar NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

// --- Loop ---
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  uint8_t res = processFingerprint();
  if (res == FINGERPRINT_OK) {
    uint16_t id = finger.fingerID;
    Serial.print("Huella reconocida! ID: ");
    Serial.println(id);

    // Obtener hora y fecha actual
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("⚠️ Error obteniendo hora");
      return;
    }

    char hora[10];
    char fecha[12];
    strftime(hora, sizeof(hora), "%H:%M:%S", &timeinfo);
    strftime(fecha, sizeof(fecha), "%Y-%m-%d", &timeinfo);

    // 🧠 Crear mensaje JSON con hora y fecha actual
    String mensaje = "{";
    mensaje += "\"numero_id\":" + String(id) + ",";
    mensaje += "\"hora\":\"" + String(hora) + "\",";
    mensaje += "\"fecha\":\"" + String(fecha) + "\"";
    mensaje += "}";

    // 📡 Publicar en MQTT
    client.publish("Sensores/Sensor_de_Huella", mensaje.c_str());
    Serial.println("📤 Enviado al broker MQTT:");
    Serial.println(mensaje);
  }

  delay(1000);
}

// --- Función auxiliar ---
uint8_t processFingerprint() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return p;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return p;

  p = finger.fingerSearch();
  return p;
}
