#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
#include "time.h"

// --- WiFi ---
char ssid[] = "TOVAR";
char pass[] = "29719073";

// --- MQTT ---
const char* mqtt_server = "161.132.38.123";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// --- Sensor de huella ---
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// --- Configuración NTP ---
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -5 * 3600; // Perú GMT-5
const int daylightOffset_sec = 0;

// --- Sensor de agua ---
int waterPin = 4;   // D2 del ESP32
int nivelAgua = 0;

// --- Sensor de humo ---
int smokePin = 2;   // DO conectado a D2 = GPIO2
int estadoHumo = 0;

// --- Sensor de proximidad (HC-SR04) ---
const int trigPin = 19;  // TRIG en D19
const int echoPin = 18;  // ECHO en D18
long duration;
float distancia;

// --- Funciones WiFi/MQTT ---
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
    if (client.connect("ESP32MultiSensores")) {
      Serial.println("✅ Conectado al broker MQTT");
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

  // Agua y humo
  pinMode(waterPin, INPUT);
  pinMode(smokePin, INPUT);

  // Proximidad
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // WiFi/MQTT
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

  // NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

// --- Loop ---
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // --- Sensor de huella ---
  uint8_t res = processFingerprint();
  if (res == FINGERPRINT_OK) {
    uint16_t id = finger.fingerID;

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("⚠️ Error obteniendo hora");
    } else {
      char hora[10];
      char fecha[12];
      strftime(hora, sizeof(hora), "%H:%M:%S", &timeinfo);
      strftime(fecha, sizeof(fecha), "%Y-%m-%d", &timeinfo);

      String msgHuella = "{ \"numero_id\": " + String(id) +
                         ", \"hora\":\"" + String(hora) +
                         "\", \"fecha\":\"" + String(fecha) + "\" }";
      client.publish("Sensores/Sensor_de_Huella", msgHuella.c_str());
      Serial.println("📤 Huella -> " + msgHuella);
    }
  }

  // --- Sensor de agua ---
  nivelAgua = digitalRead(waterPin);
  String msgAgua = "{ \"valor_agua\": " + String(nivelAgua) + " }";
  client.publish("Sensores/Sensor_de_Agua", msgAgua.c_str());
  Serial.println("📤 Agua -> " + msgAgua);

  // --- Sensor de humo ---
  estadoHumo = digitalRead(smokePin);
  String msgHumo = "{ \"valor_humo\": " + String(estadoHumo) + " }";
  client.publish("Sensores/Sensor_de_Humo", msgHumo.c_str());
  Serial.println("📤 Humo -> " + msgHumo);

  // --- Sensor de proximidad ---
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distancia = duration * 0.034 / 2;

  String msgProx = "{ \"distancia\": " + String(distancia, 2) + " }";
  client.publish("Sensores/Sensor_Proximidad", msgProx.c_str());
  Serial.println("📤 Proximidad -> " + msgProx);

  delay(1000);
}

// --- Función auxiliar huella ---
uint8_t processFingerprint() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return p;
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return p;
  p = finger.fingerSearch();
  return p;
}
