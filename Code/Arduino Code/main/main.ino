// ---------------- Header file ------------------
#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Keypad.h>
#include "wifi_AWS_config.h"
#include "pin_setup.h"
#include "functions.h"
#include <NTPClient.h> // For NTP time synchronization
#include <WiFiUdp.h>   // Required for NTPClient

// NTP Client Setup
WiFiUDP ntpUDP;
// NTP server, UTC offset (seconds), update interval (ms)
// For IST (Indian Standard Time), UTC offset is +5:30 hours = 5.5 * 3600 = 19800 seconds
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // Moved declaration here

// ------------------ Setup -------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Starting Smart Pill Dispenser...");
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

  // Initialize buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN,  HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW); // Ensure buzzer is off initially

  // Initialize compartment LED pins
  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    pinMode(compartmentLEDs[i], OUTPUT);
    digitalWrite(compartmentLEDs[i], LOW); // Ensure LEDs are off initially
  }

  // Initialize IR sensor pins (assuming active LOW when open, so use INPUT_PULLUP)
  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    pinMode(compartmentIRs[i], INPUT_PULLUP);
  }

  // Initialize compartment data (good for consistent start)
  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    compartment[i].noPillstored = 0;
    compartment[i].noPillRemaining = 0;
    for (int j = 0; j < MAX_SCHEDULES_PER_COMPARTMENT; j++) {
      compartment[i].schedule[j].active = false;
      compartment[i].schedule[j].takenToday = false;
    }
  }

  Serial.println("Setup complete. Connecting to WiFi...");
  connectWiFi(); // Connect to Wi-Fi
  
  // --- NTP Synchronization after WiFi connection ---
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(10, 10);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("Syncing Time (NTP)...");
  Serial.println("Syncing Time via NTP...");
  timeClient.begin();
  // Loop until time is successfully updated from NTP
  while(!timeClient.update()) {
    timeClient.forceUpdate(); // Force an update if the first one fails
    Serial.print(".");
    tft.print(".");
    delay(500);
  }
  rtc.adjust(DateTime(timeClient.getEpochTime())); // Adjust RTC with NTP time
  Serial.println("\nTime synced with NTP.");
  displayMessage("Time Synced!", ILI9341_GREEN, 1500);
  // --- End NTP Sync ---
  client.setServer(AWS_IOT_ENDPOINT, 8883); // Set MQTT server and port
  client.setCallback(mqttCallback); // Set MQTT message callback
  Serial.println("Setup complete. Displaying current time...");
  
}

//-------------------- loop ------------------------------------------------------------------------
void loop() {
  // Check RTC validity again (optional, usually not needed after setup)
  if (!rtc.begin()) {
    Serial.println("RTC invalid in loop!");
    delay(1000);
    return;
  }

  // Handle MQTT connection and messages
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi(); // Ensure WiFi is connected
  }

  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) { // Try to reconnect every 5 seconds
      lastReconnectAttempt = now;
      connectAWS();
    }
  }
  client.loop(); // Process incoming MQTT messages and maintain connection

  
  printCurrentTime(); // Show updated time once per second
  checkAndIndicatePills(); // Check for scheduled dispensing
  resetTakenToday(); // Reset daily taken flags at midnight

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
 if (key == '#') {
  tft.fillScreen(ILI9341_BLACK);//clear screen
  menu();// Enter menu mode
  }
  delay(100);  // Small delay for responsiveness
}
