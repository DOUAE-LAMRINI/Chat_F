#define LED 6
#define BUTTON 7

bool ledState = LOW;  
bool buttonState;
bool lastButtonState = HIGH;  

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
}

void loop() {
  buttonState = digitalRead(BUTTON);

  
  if (buttonState == LOW && lastButtonState == HIGH) {
    delay(50); 
    ledState = !ledState;  
    digitalWrite(LED, ledState);
  }

  lastButtonState = buttonState;  
}
