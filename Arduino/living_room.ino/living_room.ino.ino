#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Configuration WiFi
const char* ssid = "douae";
const char* password = "1312acab";

// Configuration MQTT
const char* mqtt_server = " 192.168.42.172"; 
const int mqtt_port = 1883;
const char* mqtt_user = ""; 
const char* mqtt_password = ""; 

// Déclaration des broches
#define DHTPIN 4
#define DHTTYPE DHT11
#define SOUND_SENSOR_PIN 34  // A0 du capteur de son
#define BUTTON_PIN 5
#define LED_PIN 2

// Variables globales
DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);
bool soundSensorEnabled = true;
unsigned long lastMsgTime = 0;
const long interval = 5000; // Intervalle d'envoi des données (5s)

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connexion à ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connecté");
  Serial.println("Adresse IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentative de connexion MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("connecté");
    } else {
      Serial.print("échec, rc=");
      Serial.print(client.state());
      Serial.println(" nouvelle tentative dans 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(SOUND_SENSOR_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  dht.begin();
  
  digitalWrite(LED_PIN, soundSensorEnabled ? HIGH : LOW);
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Gestion du bouton avec anti-rebond
  static unsigned long lastDebounceTime = 0;
  static int lastButtonState = HIGH;
  
  int reading = digitalRead(BUTTON_PIN);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > 50) {
    if (reading == LOW) {
      soundSensorEnabled = !soundSensorEnabled;
      digitalWrite(LED_PIN, soundSensorEnabled ? HIGH : LOW);
      Serial.println(soundSensorEnabled ? "Capteur de son activé" : "Capteur de son désactivé");
      delay(300);
    }
  }
  
  lastButtonState = reading;

  // Envoi des données périodiquement
  unsigned long now = millis();
  if (now - lastMsgTime > interval) {
    lastMsgTime = now;
    
    // Lecture DHT11
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Erreur de lecture du DHT11!");
    } else {
      // Affichage sur le port série
      Serial.print("Humidité: ");
      Serial.print(humidity);
      Serial.print(" %\t");
      Serial.print("Température: ");
      Serial.print(temperature);
      Serial.println(" °C");
      
      // Envoi MQTT
      char tempString[8];
      char humString[8];
      dtostrf(temperature, 1, 2, tempString);
      dtostrf(humidity, 1, 2, humString);
      
      client.publish("esp32/dht11/temperature", tempString);
      client.publish("esp32/dht11/humidity", humString);
    }

    // Lecture capteur de son si activé
    if (soundSensorEnabled) {
      int soundValue = analogRead(SOUND_SENSOR_PIN);
      Serial.print("Niveau sonore: ");
      Serial.println(soundValue);
      
      char soundString[8];
      itoa(soundValue, soundString, 10);
      client.publish("esp32/sound/level", soundString);
    } else {
      client.publish("esp32/sound/status", "disabled");
    }
  }
  
  delay(100); // Petite pause pour stabilité
}