# 🏠 Sistema de Casa Inteligente IoT (Proyecto Emergentes)

## 📖 Descripción del Proyecto
Este proyecto es un sistema de domótica e Internet de las Cosas (IoT) diseñado para monitorear y controlar los accesos y el entorno de un hogar de forma remota. Utiliza una arquitectura distribuida donde un **ESP32** se encarga de la recolección de datos mediante diversos sensores, y un **Arduino UNO R4 WiFi** actúa como controlador de los actuadores y alarmas. 

Toda la comunicación se realiza de manera inalámbrica mediante el protocolo **MQTT** conectado a un servidor VPS. La lógica de control, el registro de eventos en base de datos (**MySQL**) y la interfaz de usuario se gestionan a través de **Node-RED**, incluyendo un sistema de notificaciones automáticas por WhatsApp ante emergencias (como detección de humo).

---

## ⚙️ Requisitos del Sistema

### Hardware
* **Microcontroladores:**
  * 1x ESP32 (Módulo de desarrollo)
  * 1x Arduino UNO R4 WiFi
* **Sensores (Entradas):**
  * Sensor de gas y humo (MQ-2)
  * Sensor de agua / lluvia
  * Sensor de huella digital (AD608 – 300H)
  * Sensor ultrasónico (HC-SR04)
* **Actuadores y Periféricos (Salidas):**
  * Servomotor (SG90) para simulación de control de puerta
  * LEDs RGB para iluminación controlada
  * Buzzer activo para alertas sonoras

### Software y Tecnologías
* **Entorno de Desarrollo:** VS Code / Arduino IDE (C++)
* **Backend y Lógica:** Node-RED
* **Base de Datos:** MySQL
* **Protocolo de Comunicación:** MQTT (Broker en VPS)
* **Control de Versiones:** Git / GitHub

---

## 🏗️ Arquitectura del Sistema

```mermaid
flowchart LR
    classDef hardware fill:#e1f5fe,stroke:#0288d1,stroke-width:2px,color:#000
    classDef red fill:#fbe9e7,stroke:#d84315,stroke-width:2px,color:#000
    classDef server fill:#e8f5e9,stroke:#2e7d32,stroke-width:2px,color:#000
    classDef user fill:#fff3e0,stroke:#ef6c00,stroke-width:2px,color:#000

    subgraph CapaFisica ["1. Capa Física (Hardware Local)"]
        direction TB
        subgraph Sensores ["Sensores (Entradas)"]
            S1(Gas y Humo MQ-2)
            S2(Sensor de Agua)
            S3(Huella Digital AD608)
            S4(Ultrasonido HC-SR04)
        end

        subgraph Actuadores ["Actuadores (Salidas)"]
            A1(Servomotor SG90 - Puerta)
            A2(Leds RGB - Luces)
            A3(Buzzer - Alarma)
        end

        ESP32["ESP32\n(Gestión de Sensores)"]:::hardware
        Arduino["Arduino UNO R4 WiFi\n(Control de Actuadores)"]:::hardware

        S1 --> ESP32
        S2 --> ESP32
        S3 --> ESP32
        S4 --> ESP32

        Arduino --> A1
        Arduino --> A2
        Arduino --> A3
    end

    subgraph CapaRed ["2. Capa de Comunicación"]
        Broker{{"Broker MQTT\n(Nube / VPS)"}}:::red
    end

    subgraph CapaServidor ["3. Capa de Servidor (Lógica central)"]
        NodeRED("Node-RED\n(Reglas y Flujos)"):::server
        MySQL[("MySQL\n(Base de Datos)")]:::server
    end

    subgraph CapaUsuario ["4. Capa de Usuario (Frontend)"]
        Dashboard(("Dashboard Web\n(Interfaz Gráfica)")):::user
        WhatsApp(("Bot de WhatsApp\n(Notificaciones)")):::user
    end

    ESP32 -- "Publica lecturas\n(WiFi)" ---> Broker
    Broker -- "Envía comandos\n(WiFi)" ---> Arduino
    Broker <== "Suscripción / Publicación" ==> NodeRED
    NodeRED <== "Consulta / Inserta" ==> MySQL
    NodeRED -- "Actualiza vista" ---> Dashboard
    Dashboard -- "Acciones manuales" ---> NodeRED
    NodeRED -- "Envía Alerta de Humo" ---> WhatsApp
