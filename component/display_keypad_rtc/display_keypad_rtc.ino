#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Keypad.h>

// ------------------ TFT Setup ------------------
#define TFT_CS   5
#define TFT_DC   27
#define TFT_RST  33
#define TFT_MOSI 23
#define TFT_SCK  18
#define TFT_LED  22
#define TFT_MISO 19

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// ------------------ RTC Setup ------------------
RTC_DS3231 rtc;

// ------------------ Keypad Setup ------------------
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {32, 25, 26, 14}; // Connect to keypad ROW0, ROW1, ROW2, ROW3
byte colPins[COLS] = {13, 12, 21, 15}; // Connect to keypad COL0, COL1, COL2, COL3

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ------------------ Time Tracker ------------------
int lastSecond = -1;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting RTC...");

  Wire.begin(); // Ensure I2C is initialized

  // TFT Init
  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(1); // Landscape
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);

  // RTC Init
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);  // Halt
  }

  // If RTC lost power, reset the time
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time to compile time...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.println("Setup complete.");
}

// ------------------ Function to Print Time ------------------
void printCurrentTime() {
  DateTime now = rtc.now();

  // Only update if the second has changed
  if (now.second() != lastSecond) {
    lastSecond = now.second();

    String day = String(now.day()) + "/" + String(now.month()) + "/" + String(now.year());
    String time = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());

    Serial.println(day);
    Serial.println(time);

    // Clear previous time display
    tft.fillRect(0, 0, 320, 60, ILI9341_BLACK);
    
    tft.setCursor(10, 10);
    tft.setTextSize(3);
    tft.setTextColor(ILI9341_GREEN);
    tft.print("Time: ");
    tft.print(time);
    
    tft.setCursor(10, 45);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_CYAN);
    tft.print("Date: ");
    tft.print(day);
  }
}

void loop() {
  // Check RTC validity again (optional, usually not needed after setup)
  if (!rtc.begin()) {
    Serial.println("RTC invalid in loop!");
    delay(1000);
    return;
  }

  // Show updated time once per second
  printCurrentTime();

  // Keypad logic
  char key = keypad.getKey();
  if (key) {
    Serial.print("Key Pressed: ");
    Serial.println(key);

    tft.fillRect(10, 100, 300, 40, ILI9341_BLACK);
    tft.setCursor(10, 100);
    tft.setTextSize(3);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print("Key: ");
    tft.print(key);
  }

  delay(100);  // Small delay for responsiveness
}
