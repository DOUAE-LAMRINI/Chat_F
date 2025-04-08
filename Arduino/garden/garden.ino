int soilMoisturePin = A0;  // Capteur d'humidité de sol (analogique)
int waterLevelPin = D2;    // Capteur de niveau d'eau (digital)
int pumpPin = D3;          // Pompe (relais)

void setup() {
  Serial.begin(115200);

  pinMode(soilMoisturePin, INPUT);
  pinMode(waterLevelPin, INPUT);
  pinMode(pumpPin, OUTPUT);

  digitalWrite(pumpPin, LOW); // La pompe est éteinte au départ
}

void loop() {
  int soilValue = analogRead(soilMoisturePin); // 0 = humide, 1023 = sec
  int waterLevel = digitalRead(waterLevelPin); // HIGH ou LOW

  Serial.print("Humidité sol : ");
  Serial.println(soilValue);

  Serial.print("Niveau d'eau : ");
  Serial.println(waterLevel == HIGH ? "OK" : "BAS");

  // Exemple de seuil : si solValue > 700, le sol est trop sec
  if (soilValue > 600 && waterLevel == HIGH) {
    Serial.println("=> Sol sec et niveau OK : activation de la pompe");
    digitalWrite(pumpPin, HIGH);  // Active la pompe
  } else {
    Serial.println("=> Pas besoin d’arroser / niveau trop bas");
    digitalWrite(pumpPin, LOW);   // Éteint la pompe
  }

  delay(3000); // Attendre 3 secondes avant prochaine mesure
}
