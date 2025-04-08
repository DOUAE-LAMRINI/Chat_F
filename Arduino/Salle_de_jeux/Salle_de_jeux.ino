// Définition des broches pour la LED RGB
const int redPin = 8;
const int greenPin = 9;
const int bluePin = 10;

// Définition des broches pour les boutons
const int buttonRed = 5;
const int buttonGreen = 4;
const int buttonBlue = 3;
const int buttonMulti = 2;

// Variables d'état des couleurs
bool redState = false;
bool greenState = false;
bool blueState = false;
bool multiColorMode = false;

// Variables pour gérer l'anti-rebond
bool lastButtonRedState = HIGH;
bool lastButtonGreenState = HIGH;
bool lastButtonBlueState = HIGH;
bool lastButtonMultiState = HIGH;

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(buttonRed, INPUT_PULLUP);
  pinMode(buttonGreen, INPUT_PULLUP);
  pinMode(buttonBlue, INPUT_PULLUP);
  pinMode(buttonMulti, INPUT_PULLUP);
}

void loop() {
  handleButtonToggle(buttonRed, lastButtonRedState, redState);
  handleButtonToggle(buttonGreen, lastButtonGreenState, greenState);
  handleButtonToggle(buttonBlue, lastButtonBlueState, blueState);
  handleButtonToggle(buttonMulti, lastButtonMultiState, multiColorMode);

  if (multiColorMode) {
    rainbowEffect();
  } else {
    digitalWrite(redPin, redState ? HIGH : LOW);
    digitalWrite(greenPin, greenState ? HIGH : LOW);
    digitalWrite(bluePin, blueState ? HIGH : LOW);
  }
}

void handleButtonToggle(int buttonPin, bool &lastButtonState, bool &colorState) {
  bool buttonPressed = digitalRead(buttonPin) == LOW;
  if (buttonPressed && lastButtonState == HIGH) {
    colorState = !colorState;
    delay(300); // Anti-rebond
  }
  lastButtonState = buttonPressed;
}

void rainbowEffect() {
  for (int i = 0; i < 255; i++) {
    analogWrite(redPin, i);
    analogWrite(greenPin, 255 - i);
    analogWrite(bluePin, i / 2);
    checkMultiColorButton(); // Vérifie si le bouton multi est pressé
    delay(10);
  }
  for (int i = 255; i > 0; i--) {
    analogWrite(redPin, i / 2);
    analogWrite(greenPin, i);
    analogWrite(bluePin, 255 - i);
    checkMultiColorButton(); // Vérifie si le bouton multi est pressé
    delay(10);
  }
}

void checkMultiColorButton() {
  bool buttonPressed = digitalRead(buttonMulti) == LOW;
  if (buttonPressed) {
    multiColorMode = false; // Désactive le mode multicolore
    delay(300); // Anti-rebond
  }
}