#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Keypad.h>

// TFT pins (confirm your wiring)
#define TFT_CS   5   
#define TFT_RST  33  
#define TFT_DC   27  
#define TFT_MOSI 23  
#define TFT_SCLK 18  
#define TFT_LED  22  
#define TFT_MISO 19 

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST, TFT_MISO);

RTC_DS3231 rtc;

// Keypad setup (your original config)
const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {32, 25, 26, 14};
byte colPins[COLS] = {13, 12, 21, 15};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Days of week
const char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

int lastSecond = -1;

void printCurrentTime() {
  DateTime now = rtc.now();

  if (!now.isValid()) {
    tft.fillRect(0, 30, tft.width(), 50, ILI9341_BLACK);
    tft.setCursor(0, 30);
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(2);
    tft.print("RTC INVALID TIME");
    return;
  }

  // Format date string
  String dateStr = String(daysOfTheWeek[now.dayOfTheWeek()]) + " " +
                   String(now.year()) + "-" +
                   (now.month() < 10 ? "0" : "") + String(now.month()) + "-" +
                   (now.day() < 10 ? "0" : "") + String(now.day());

  // Format time string
  String timeStr = (now.hour() < 10 ? "0" : "") + String(now.hour()) + ":" +
                   (now.minute() < 10 ? "0" : "") + String(now.minute()) + ":" +
                   (now.second() < 10 ? "0" : "") + String(now.second());

  // Clear display area for time
  tft.fillRect(0, 30, tft.width(), 50, ILI9341_BLACK);

  // Print date
  tft.setCursor(0, 30);
  tft.setTextColor(ILI9341_CYAN);
  tft.setTextSize(2);
  tft.print(dateStr);

  // Print time on next line (y + 30)
  tft.setCursor(0, 60);
  tft.print(timeStr);
}

void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22); // Adjust SDA, SCL pins if needed

  // TFT init
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(0, 0);
  tft.println("Welcome to SMB");

  Serial.println("Starting RTC...");
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC!");
    while (1) delay(10);
  }
  Serial.println("RTC found.");

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  rtc.disable32K();

  printCurrentTime();

  Serial.println("Setup done.");
}

void loop() {
  DateTime now = rtc.now();

  if (!now.isValid()) {
    Serial.println("RTC time invalid!");
    delay(500);
    return;
  }

  if (now.second() != lastSecond) {
    printCurrentTime();
    lastSecond = now.second();
  }

  char key = customKeypad.getKey();
  if (key) {
    Serial.print("Key Pressed: ");
    Serial.println(key);

    tft.fillRect(0, 130, tft.width(), 30, ILI9341_BLACK);
    tft.setCursor(0, 130);
    tft.setTextColor(ILI9341_GREEN);
    tft.setTextSize(2);
    tft.print("Pressed: ");
    tft.print(key);
  }

  delay(100); // small delay for stability
}
