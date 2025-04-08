#define PIR_SENSOR 8
#define LED_WHITE 6
#define BUTTON 7
#define LDR_SENSOR A0

bool ledState = false;
bool lastButtonState = false;
int lightThreshold = 500;  // Adjust based on your LDR readings

void setup() {
  pinMode(PIR_SENSOR, INPUT);
  pinMode(LED_WHITE, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LDR_SENSOR, INPUT);
  Serial.begin(9600);
}

void loop() {
  // Button handling (always works)
  bool currentButtonState = digitalRead(BUTTON);
  if (currentButtonState != lastButtonState) {
    delay(50); // Debounce
    if (currentButtonState == LOW) {
      ledState = !ledState;
      digitalWrite(LED_WHITE, ledState);
    }
  }
  lastButtonState = currentButtonState;

  // Only check motion if LED isn't already manually turned on
  if (!ledState) {
    int lightLevel = analogRead(LDR_SENSOR);
    bool motionDetected = digitalRead(PIR_SENSOR);

    // Turn on LED ONLY if dark AND motion detected
    if (lightLevel < lightThreshold && motionDetected) {
      digitalWrite(LED_WHITE, HIGH);
      ledState = true;
    }
  }
}