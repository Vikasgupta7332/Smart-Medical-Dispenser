#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc; // Create an instance of the RTC_DS3231 class

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  // Uncomment the following lines to set the initial date and time if needed
   rtc.adjust(DateTime(__DATE__, __TIME__));
}

void loop() {
  // Read the current date and time from the DS3231
  DateTime now = rtc.now();
  
  // Print the date and time on the same line with two digits for hours, minutes, and seconds
  Serial.print("Current Date: ");
  Serial.print(now.year(), DEC);
  Serial.print("/");
  printTwoDigits(now.month());
  Serial.print("/");
  printTwoDigits(now.day());
  Serial.print("  Current Time: ");
  printTwoDigits(now.hour());
  Serial.print(":");
  printTwoDigits(now.minute());
  Serial.print(":");
  printTwoDigits(now.second());
  Serial.println();

  delay(1000); // Update every 1 second
}

void printTwoDigits(int number) {
  if (number < 10) {
    Serial.print("0"); // Print a leading zero for single-digit numbers
  }
  Serial.print(number);
}
