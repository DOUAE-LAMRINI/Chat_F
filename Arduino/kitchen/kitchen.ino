#include <WiFi.h>
#include <PubSubClient.h>

#define WHITE_LED 13
#define RED_LED 4
#define BUZZER 2
#define BUTTON 27
#define FLAME_SENSOR 32
#define MQ3_PIN 35

const char* ssid = "douae";
const char* password = "1312acab";
const char* mqtt_server = "192.168.6.172";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;

void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("GasFlameSensor")) {
      Serial.println("connected");
      client.subscribe("esp32/commands");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup() {
  pinMode(WHITE_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(FLAME_SENSOR, INPUT);
  
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqtt_callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 500) {
    lastMsg = now;

    // Button control
    static bool lastButtonState = HIGH;
    if (digitalRead(BUTTON) == LOW && lastButtonState == HIGH) {
      delay(50);
      if (digitalRead(BUTTON) == LOW) {
        bool currentState = digitalRead(WHITE_LED);
        digitalWrite(WHITE_LED, !currentState);
        client.publish("esp32/led/white", !currentState ? "ON" : "OFF");
      }
    }
    lastButtonState = digitalRead(BUTTON);

    // Sensor readings
    int gasLevel = analogRead(MQ3_PIN);
    bool flameDetected = (digitalRead(FLAME_SENSOR) == LOW);
    
    // Publish to MQTT
    client.publish("esp32/gas/level", String(gasLevel).c_str());
    client.publish("esp32/flame/status", flameDetected ? "DETECTED" : "SAFE");

    // Local alerts
    if (flameDetected || gasLevel > 2000) {
      digitalWrite(RED_LED, HIGH);
      tone(BUZZER, 1000);
      client.publish("esp32/alert", 
        flameDetected ? "FIRE" : String("GAS:" + String(gasLevel)).c_str());
    } else {
      digitalWrite(RED_LED, LOW);
      noTone(BUZZER);
    }
  }
}