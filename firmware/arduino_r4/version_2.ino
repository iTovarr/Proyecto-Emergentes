#include <WiFiS3.h>
#include <PubSubClient.h>
#include <Servo.h>

// --- WiFi ---
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

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

// --- Relay ---
const int relayPin = 4;

// --- Servo ---
Servo servoMotor; // pin 5

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

  // --- Control de luces ---
  if (String(topic) == "Luces/Sala")          digitalWrite(salaPin,    mensaje == "luz prendida");
  if (String(topic) == "Luces/Garaje")        digitalWrite(garajePin,  mensaje == "luz prendida");
  if (String(topic) == "Luces/Cocina")        digitalWrite(cocinaPin,  mensaje == "luz prendida");
  if (String(topic) == "Luces/Sala_Comedor")  digitalWrite(comedorPin, mensaje == "luz prendida");
  if (String(topic) == "Luces/Cuarto")        digitalWrite(cuartoPin,  mensaje == "luz prendida");

  // --- Control del buzzer (alarma) ---
  if (String(topic) == "Sistema/Alarma" && mensaje == "Alarma_Activada") {
    digitalWrite(buzzerPin, HIGH);
    delay(5000);
    digitalWrite(buzzerPin, LOW);
  }

  // --- Control de puerta (servo) ---
  if (String(topic) == "Sistema/Puerta" && mensaje == "Puerta_Abierta") {
    Serial.println("🚪 Puerta abierta");
    servoMotor.write(90);   // mover a 90°
    delay(3000);            // esperar 3 segundos
    servoMotor.write(0);    // regresar a 0°
    Serial.println("🚪 Puerta cerrada");
  }

  // --- Control de techo (relay) ---
  if (String(topic) == "Sistema/Humedad" && mensaje == "Techo_Activado") {
    digitalWrite(relayPin, LOW);   // activar relay
    delay(5000);                   // mantener 5 segundos
    digitalWrite(relayPin, HIGH);  // desactivar relay
  }
}

// --- Reconexión MQTT ---
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ArduinoR4LucesAlarmaServoRelay")) {
      Serial.println("✅ Conectado al broker MQTT");
      client.subscribe("Luces/Sala");
      client.subscribe("Luces/Garaje");
      client.subscribe("Luces/Cocina");
      client.subscribe("Luces/Sala_Comedor");
      client.subscribe("Luces/Cuarto");
      client.subscribe("Sistema/Alarma");
      client.subscribe("Sistema/Puerta");
      client.subscribe("Sistema/Humedad");
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
  pinMode(relayPin, OUTPUT);

  digitalWrite(salaPin, LOW);
  digitalWrite(garajePin, LOW);
  digitalWrite(cocinaPin, LOW);
  digitalWrite(comedorPin, LOW);
  digitalWrite(cuartoPin, LOW);
  digitalWrite(buzzerPin, LOW);
  digitalWrite(relayPin, HIGH); // relay apagado por defecto

  servoMotor.attach(5);
  servoMotor.write(0); // servo inicial en 0°

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