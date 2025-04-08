#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>

// LCD Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Keypad Setup
const byte ROWS = 4;  // 4 Rows
const byte COLS = 4;  // 4 Columns

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {2, 3, 4, 5};  // Connected to the row pins of the keypad
byte colPins[COLS] = {6, 7, 8, 9};  // Connected to the column pins

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Servo Setup
Servo doorServo;
const int servoPin = 10;  // Pin connected to servo
const int servoClosedPos = 0;  // Closed position
const int servoOpenPos = 90;    // Open position

// Password Settings
const String correctPassword = "5678";
String enteredPassword = "";
int attemptsLeft = 3;
bool isLocked = false;
unsigned long lockTime = 0;
const long lockDuration = 10000;  // 10 seconds lockout
unsigned long lastKeyPressTime = 0;
const int keyPressDelay = 500;  // 500ms delay to prevent fast input errors

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  doorServo.attach(servoPin);
  doorServo.write(servoClosedPos);
  displayEnterCode();
}

void loop() {
  if (isLocked) {
    handleLockout();
    return;
  }

  char key = keypad.getKey();
  if (key) {
    lastKeyPressTime = millis();
    Serial.print("Key Pressed: ");
    Serial.println(key);

    if (key == '*') {
      resetInput();  // Clear input if '*' is pressed
    } else if (isdigit(key)) {
      handleDigitInput(key);
    }
  }

  // Process password after a short delay to prevent rapid invalid submissions
  if (enteredPassword.length() == 4 && (millis() - lastKeyPressTime) > keyPressDelay) {
    checkPassword();
  }
}

void handleDigitInput(char key) {
  if (enteredPassword.length() < 4) {
    enteredPassword += key;
    lcd.setCursor(enteredPassword.length() - 1, 1);
    lcd.print('*');  // Mask input with '*'
  }
}

void checkPassword() {
  Serial.print("Entered: "); Serial.println(enteredPassword);
  Serial.print("Correct: "); Serial.println(correctPassword);

  if (enteredPassword == correctPassword) {
    grantAccess();
  } else {
    denyAccess();
  }
}

void grantAccess() {
  lcd.clear();
  lcd.print("Access Granted!");
  doorServo.write(servoOpenPos);
  delay(10000);
  doorServo.write(servoClosedPos);
  resetInput();
  displayEnterCode();
}

void denyAccess() {
  attemptsLeft--;
  lcd.clear();
  lcd.print("Wrong Code!");
  lcd.setCursor(0, 1);
  lcd.print("Attempts left: ");
  lcd.print(attemptsLeft);
  delay(2000);

  if (attemptsLeft <= 0) {
    activateLockout();
  } else {
    resetInput();
    displayEnterCode();
  }
}

void activateLockout() {
  isLocked = true;
  lockTime = millis();
  displayCountdown();
}

void handleLockout() {
  unsigned long elapsed = millis() - lockTime;
  if (elapsed >= lockDuration) {
    isLocked = false;
    attemptsLeft = 3;
    displayEnterCode();
  } else {
    displayCountdown();
  }
}

void displayCountdown() {
  lcd.clear();
  lcd.print("System Locked!");
  int remaining = (lockDuration - (millis() - lockTime)) / 1000 + 1;
  lcd.setCursor(0, 1);
  lcd.print("Wait: ");
  lcd.print(remaining);
  lcd.print("s   ");
}

void resetInput() {
  enteredPassword = "";
  lcd.setCursor(0, 1);
  lcd.print("                ");  // Clear display row
}

void displayEnterCode() {
  lcd.clear();
  lcd.print("Enter code:");
  lcd.setCursor(0, 1);
}
