#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <DHT.h>

// TFT Pins (Arduino Uno)
#define TFT_CS   7
#define TFT_RST  8
#define TFT_DC   9

// Sensor Pins
#define DHTPIN  2
#define DHTTYPE DHT11
#define MG_PIN  A0

DHT dht(DHTPIN, DHTTYPE);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(9600);
  dht.begin();
  
  // Initialize TFT in LANDSCAPE mode
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST7735_WHITE);

  drawUI();
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int co2 = analogRead(MG_PIN);

  // Clear dynamic areas before updating
  tft.fillRect(85, 50, 50, 10, ST7735_WHITE);  // CO2 Value area
  tft.fillRect(85, 90, 50, 10, ST7735_WHITE);  // Temperature Value area
  tft.fillRect(85, 110, 50, 10, ST7735_WHITE); // Humidity Value area

  // Display CO2 Value
  tft.setCursor(85, 50);
  tft.setTextColor(ST7735_GREEN);
  tft.setTextSize(1);
  tft.print("= ");
  tft.print(co2);

  // Display Temperature Value
  tft.setCursor(85, 90);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(1);
  tft.print(t, 1);
  tft.print(" C");

  // Display Humidity Value
  tft.setCursor(85, 110);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(1);
  tft.print(h, 1);
  tft.print(" %");

  delay(2000);
}

void drawUI() {
  // Welcome Text
  tft.setTextColor(ST7735_BLUE);
  tft.setTextSize(2);
  tft.setCursor(25, 5);
  tft.print("Welcome");

  // Subtitle
  tft.setTextSize(1);
  tft.setCursor(15, 25);
  tft.print("to our Smart building");

  // CO2 Label
  tft.setCursor(60, 50);
  tft.setTextColor(ST7735_GREEN);
  tft.setTextSize(1);
  tft.print("CO2");

  // Temperature Label
  tft.setCursor(60, 90);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(1);
  tft.print("T :");

  // Humidity Label
  tft.setCursor(60, 110);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(1);
  tft.print("H :");
}
