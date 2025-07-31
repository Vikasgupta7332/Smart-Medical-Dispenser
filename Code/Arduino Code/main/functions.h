
// ------------------ Time Tracker ------------------
int lastSecond = -1;
int lastDay = -1; // To track day changes for resetting 'takenToday'

//--------------------Structure of compartment data---------------------------------------------------
const byte NUM_COMPARTMENTS = 4; // Number of pill compartments (A, B, C, D)
const byte MAX_SCHEDULES_PER_COMPARTMENT = 6; // Max schedules per compartment

struct MedicineSchedule{
    int hour; // Time to take medicine (hour)
    int minute; // Time to take medicine(minute)
    int doseCount; // Number of pills per dose
    bool takenToday; // To track if pill was already taken today
    bool active; // Schedule enabled
};

struct PillCompartment {
  int noPillstored;// Number of pills stored in particular compartment
  int noPillRemaining;// Number of remaining pills in a particular compartment 
  struct MedicineSchedule schedule[MAX_SCHEDULES_PER_COMPARTMENT]; // schedule per compartment A (0), B (1), C (2), D (3)  
};

struct PillCompartment compartment[NUM_COMPARTMENTS];// Compartments A(0), B(1), C(2), D(3)


//----------------- function prototypes ---------------
void printCurrentTime();
void refillCompartment();
void schedule();
void menu();
void resetMode();
void resetTakenToday();
void checkAndIndicatePills();
void displayMessage(String message, int color = ILI9341_WHITE, int delayMs = 1500);
String readNumberInput(int maxLen, int x, int y, int maxVal = -1);
bool isCompartmentOpen(int compartmentIndex); // prototype for IR sensor check
void viewManageSchedules();
void displayCompartmentSchedules(int compIndex);

// AWS IoT related functions
void connectWiFi();
void connectAWS();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void publishPillStatus(int compIndex, String status, int doseCount, DateTime timestamp);

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
    tft.println("\n \n Press '#' for menu");
  }
}

// ------------------ Helper for reading numeric input from keypad ------------------
String readNumberInput(int maxLen, int x, int y, int maxVal) {
    String input = "";
    tft.fillRect(x, y, 200, 30, ILI9341_BLACK); // Clear input area
    tft.setCursor(x, y);
    tft.setTextColor(ILI9341_YELLOW);
    tft.setTextSize(2);

    while (true) {
        char key = keypad.getKey();
        if (key != NO_KEY) {
            if (key >= '0' && key <= '9') {
                if (input.length() < maxLen) {
                    input += key;
                    tft.print(key);
                } else {
                    displayMessage("Max digits reached!", ILI9341_RED, 800);
                    tft.setCursor(x + input.length() * 12, y); // Reposition cursor
                }
            } else if (key == '*') { // Enter key
                if (input.length() > 0) {
                    int val = input.toInt();
                    if (maxVal != -1 && val > maxVal) {
                        displayMessage("Value too high! Max " + String(maxVal), ILI9341_RED, 1500);
                        input = ""; // Clear input for re-entry
                        tft.fillRect(x, y, 200, 30, ILI9341_BLACK); // Clear input area
                        tft.setCursor(x, y);
                    } else {
                        return input;
                    }
                } else {
                    displayMessage("Enter a number!", ILI9341_RED, 1000);
                }
            } else if (key == '#') { // Cancel key
                displayMessage("Cancelled.", ILI9341_ORANGE, 1000);
                return ""; // Return empty string to indicate cancellation
            }
        }
        delay(50); // Small delay to debounce keypad
    }
}

// ------------------ Generic message display function ------------------
void displayMessage(String message, int color, int delayMs) {
    tft.fillRect(0, 200, 320, 40, ILI9341_BLACK); // Clear message area
    tft.setCursor(10, 200);
    tft.setTextSize(2);
    tft.setTextColor(color);
    tft.println(message);
    delay(delayMs);
    tft.fillRect(0, 200, 320, 40, ILI9341_BLACK); // Clear message after delay
}

//---------------------- refill function -------------------------------------------------------------

void refillCompartment(){
  char key = '\0';
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(10, 10);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("Refill Mode");
  tft.setCursor(10, 40);
  tft.println("Select Compartment A/B/C/D:");

  while ((key = keypad.getKey()) == NO_KEY); // Wait for key press
  Serial.print("Key Pressed: ");
  Serial.println(key);

  int compartmentIndex = -1;
  if (key == 'A') compartmentIndex = 0;
  else if (key == 'B') compartmentIndex = 1;
  else if (key == 'C') compartmentIndex = 2;
  else if (key == 'D') compartmentIndex = 3;
  else {
    displayMessage("Invalid Compartment!", ILI9341_RED);
    return;
  }

  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(10, 10);
  tft.printf("Compartment %c Selected.\n", key);
  tft.setCursor(10, 40);
  tft.println("Enter # of pills (then *):");
  tft.setCursor(10, 70); // Position for input

  String pillInput = readNumberInput(3, 10, 70, 999); // Max 3 digits, max 999 pills

  if (pillInput == "") { // If input was cancelled
      return;
  }

  compartment[compartmentIndex].noPillstored = pillInput.toInt();
  compartment[compartmentIndex].noPillRemaining = compartment[compartmentIndex].noPillstored; // Initialize remaining pills

  displayMessage("Compartment " + String(key) + " refilled with " + pillInput + " pills.", ILI9341_GREEN);
  delay(500); // Short delay before returning to menu
};

//---------------------- schedule function -----------------------------------------------------------

void schedule(){
   char compKey = '\0';
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(10, 10);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("Schedule Mode");
  tft.println("Select Compartment A/B/C/D:");

  while ((compKey = keypad.getKey()) == NO_KEY); // Wait for key press
  Serial.print("Key Pressed: ");
  Serial.println(compKey);

  int compIndex = -1;
  if (compKey == 'A') compIndex = 0;
  else if (compKey == 'B') compIndex = 1;
  else if (compKey == 'C') compIndex = 2;
  else if (compKey == 'D') compIndex = 3;
  else {
    displayMessage("Invalid Compartment!", ILI9341_RED);
    return;
  }

  int schedSlot = -1; // Initialize to -1 to indicate no slot found
  for (int i = 0; i < MAX_SCHEDULES_PER_COMPARTMENT; i++) {
    if (!compartment[compIndex].schedule[i].active) {
      schedSlot = i;
      break;
    }
  }

  if (schedSlot == -1) {
    // Corrected type from ILI9431_RED to ILI9341_RED
    displayMessage("No free schedule slots for " + String(compKey) + "!", ILI9341_RED);
    return;
  }

  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(10, 10);
  tft.printf("Compartment %c - New Schedule\n", compKey);
  tft.setCursor(10, 40);
  tft.println("Enter hour (00-23, then *):");
  tft.setCursor(10, 70);
  String hourStr = readNumberInput(2, 10, 70, 23); // Max 2 digits, max 23

  if (hourStr == "") return; // If cancelled

  tft.setCursor(10, 100);
  tft.println("Enter minute (00-59, then *):");
  tft.setCursor(10, 130);
  String minStr = readNumberInput(2, 10, 130, 59); // Max 2 digits, max 59

  if (minStr == "") return; // If cancelled

  tft.setCursor(10, 160);
  tft.println("Enter Dose Count (then *):");
  tft.setCursor(10, 190);
  String doseStr = readNumberInput(2, 10, 190, 99); // Max 2 digits, max 99 pills per dose

  if (doseStr == "") return; // If cancelled

  compartment[compIndex].schedule[schedSlot].hour = hourStr.toInt();
  compartment[compIndex].schedule[schedSlot].minute = minStr.toInt();
  compartment[compIndex].schedule[schedSlot].doseCount = doseStr.toInt();
  compartment[compIndex].schedule[schedSlot].active = true;
  compartment[compIndex].schedule[schedSlot].takenToday = false;

  String formattedHour = (hourStr.toInt() < 10 ? "0" : "") + hourStr;
  String formattedMin = (minStr.toInt() < 10 ? "0" : "") + minStr;

  displayMessage("Scheduled for " + String(compKey) + " at " + formattedHour + ":" + formattedMin +
                 " for " + doseStr + " pills.", ILI9341_GREEN, 2500);
};

//--------------------- Reset Mode function ---------------------------------------------------------------
void resetMode() {
    char key = '\0';
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(10, 10);
    tft.setTextSize(3);
    tft.setTextColor(ILI9341_GREEN);
    tft.println("Reset Mode:-");

    tft.setCursor(5, 45);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_CYAN);
    tft.println("1. Clear All Schedules\n");
    tft.println("2. Clear All Pills Count\n");
    tft.println("3. Back to Menu\n");
    tft.print("Enter Your Choice:");

    while ((key = keypad.getKey()) == NO_KEY);

    switch (key) {
        case '1':
            for (int comp = 0; comp < NUM_COMPARTMENTS; comp++) {
                for (int sched = 0; sched < MAX_SCHEDULES_PER_COMPARTMENT; sched++) {
                    compartment[comp].schedule[sched].active = false;
                    compartment[comp].schedule[sched].takenToday = false;
                }
            }
            displayMessage("All Schedules Cleared!", ILI9341_YELLOW);
            break;
        case '2':
            for (int comp = 0; comp < NUM_COMPARTMENTS; comp++) {
                compartment[comp].noPillstored = 0;
                compartment[comp].noPillRemaining = 0;
            }
            displayMessage("All Pill Counts Cleared!", ILI9341_YELLOW);
            break;
        case '3':
            displayMessage("Exiting Reset Mode.", ILI9341_WHITE);
            break;
        default:
            displayMessage("Invalid Input!", ILI9341_RED);
            break;
    }
    delay(500); // Short delay before returning
}

// ------------------ Authentication Function ------------------
const String AUTH_PASSWORD = "1234"; // Define a simple password for menu access
bool authenticateUser() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(10, 50);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.println("Enter Password (then *):");
  tft.setCursor(10, 80);

  String enteredPassword = "";
  // Use a loop to read password characters, displaying '*' for each digit
  while (true) {
    char key = keypad.getKey();
    if (key != NO_KEY) {
      if (key >= '0' && key <= '9') {
        if (enteredPassword.length() < AUTH_PASSWORD.length()) { // Limit input length to password length
          enteredPassword += key;
          tft.print("*"); // Display asterisk for each character
        } else {
          displayMessage("Max digits reached!", ILI9341_RED, 800);
          tft.setCursor(10 + enteredPassword.length() * 12, 80); // Reposition cursor
        }
      } else if (key == '*') { // Enter key
        break;
      } else if (key == '#') { // Cancel key
        displayMessage("Authentication Cancelled.", ILI9341_ORANGE, 1500);
        return false;
      }
    }
    delay(50); // Debounce
  }

  if (enteredPassword == AUTH_PASSWORD) {
    displayMessage("Authentication Successful!", ILI9341_GREEN, 1500);
    return true;
  } else {
    displayMessage("Incorrect Password!", ILI9341_RED, 1500);
    return false;
  }
}


//--------------------- Menu function ---------------------------------------------------------------

void menu(){
  // Authenticate user before showing the menu
  if (!authenticateUser()) {
    tft.fillScreen(ILI9341_BLACK); // Clear screen if authentication fails
    return; // Exit menu function
  }
  int lock=1;
  char key = '\0'; // Declare here
  do{
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(10, 10);
    tft.setTextSize(3);
    tft.setTextColor(ILI9341_GREEN);
    tft.print("Menu:- ");
    
    tft.setCursor(5, 45);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_CYAN);
    tft.println("1. Refill Mode\n");
    tft.println("2. Schedule Mode\n");
    tft.println("3. Reset Mode\n");
    tft.println("4. View/Manage Schedules\n");
    tft.println("5. Exit\n");
    tft.print("Enter Your Choice:");

    while ((key = keypad.getKey()) == NO_KEY); // Wait for key press

    Serial.print("Key Pressed: ");
    Serial.println(key);

    tft.fillRect(10, 190, 300, 30, ILI9341_BLACK);
    tft.setCursor(10, 190);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print("Key: ");
    tft.print(key);
    
    switch(key){
      case '1':
        // refill logic
        refillCompartment();
        break;
        
      case '2':
        // Add schedule logic
        schedule();
        break;
        
      case '3':
        resetMode();
        break;

      case '4': // Handle new menu option
        viewManageSchedules();
        break;
  
      case '5':
        tft.setCursor(10, 220);
        tft.println("Exit from menu");
        lock=0;
        tft.fillScreen(ILI9341_BLACK);//clear screen
        break;
        
      default:
        tft.setCursor(10, 220);
        tft.println("Invalid Input");
        break; 
    }
    delay(500); // Add a slight pause after each input
  } while(lock);
}

// ------------------ Function to Reset 'takenToday' at Midnight ------------------
void resetTakenToday() {
    DateTime now = rtc.now();

    // Check if the day has changed since the last check
    if (now.day() != lastDay) {
        // This condition ensures it only runs once per day, and not on initial boot if lastDay is -1
        if (lastDay != -1) {
            Serial.println("New day detected. Resetting 'takenToday' flags.");
            for (int comp = 0; comp < NUM_COMPARTMENTS; comp++) {
                for (int sched = 0; sched < MAX_SCHEDULES_PER_COMPARTMENT; sched++) {
                    compartment[comp].schedule[sched].takenToday = false;
                }
            }
            displayMessage("Daily schedule reset!", ILI9341_YELLOW, 1000);
        }
        lastDay = now.day(); // Update lastDay to current day
    }
}

// ------------------ Function to check if compartment is open using IR sensor ------------------
// Returns true if compartment is open (IR sensor detects LOW), false otherwise
bool isCompartmentOpen(int compartmentIndex) {
  // Assuming IR sensor outputs LOW when open/detected, HIGH when closed/not detected
  return digitalRead(compartmentIRs[compartmentIndex]) == LOW;
}

// ------------------ Function to Check and Dispense Pills ------------------
void checkAndIndicatePills() {
     DateTime now = rtc.now();
    // Declare scheduledTimeForLog at a scope accessible by both if and else branches
    DateTime scheduledTimeForLog; 

    for (int comp = 0; comp < NUM_COMPARTMENTS; comp++) { // Iterate through compartments
        for (int sched = 0; sched < MAX_SCHEDULES_PER_COMPARTMENT; sched++) { // Iterate through schedules
            MedicineSchedule* currentSchedule = &compartment[comp].schedule[sched];

            // Check if schedule is active, not taken today, and time matches
            if (currentSchedule->active && !currentSchedule->takenToday &&
                now.hour() == currentSchedule->hour &&
                now.minute() == currentSchedule->minute &&
                now.second() == 0) { // Trigger only at the start of the minute

                  // Assign scheduled time for logging purposes
                scheduledTimeForLog = DateTime(now.year(), now.month(), now.day(),currentSchedule->hour, currentSchedule->minute, 0);
                                                        
                // It's time to dispense!
                if (compartment[comp].noPillRemaining >= currentSchedule->doseCount) {
                    // --- ALARM NOTIFICATION ---
                    digitalWrite(BUZZER_PIN, HIGH); // Turn on buzzer
                    digitalWrite(compartmentLEDs[comp], HIGH); // Turn on compartment LED
                    tft.fillScreen(ILI9341_RED);
                    tft.setCursor(10, 30); // Adjusted Y position for more space
                    tft.setTextSize(3);
                    tft.setTextColor(ILI9341_WHITE);
                    tft.println("TIME FOR MEDS!");

                    // Indicate the compartment prominently
                    tft.setCursor(10, 80);
                    tft.setTextSize(2); // Larger text for compartment letter
                    tft.setTextColor(ILI9341_YELLOW); // Different color for compartment
                    tft.printf("COMPARTMENT %c\n", 'A' + comp);

                    tft.setCursor(10, 140); // Adjusted Y position
                    tft.setTextSize(3);
                    tft.setTextColor(ILI9341_WHITE);
                    tft.printf("Dose: %d pills\n", currentSchedule->doseCount);

                    tft.setCursor(10, 200); // Adjusted Y position
                    tft.setTextSize(2);
                    tft.println("Press any key to dismiss");

                    // Wait for user to dismiss alarm (or timeout)
                    unsigned long alarmStartTime = millis();
                    bool dismissed = false;
                    while (!dismissed && (millis() - alarmStartTime < 15000)) { // Alarm for 15 seconds
                        if (keypad.getKey() != NO_KEY) {
                            dismissed = true;
                        }
                        delay(50); // Small delay to prevent busy-waiting
                    }
                    delay(1000);//delay for 1 second
                    digitalWrite(BUZZER_PIN, LOW); // Turn off buzzer

                    // --- WAIT FOR COMPARTMENT TO BE OPENED ---
                    tft.fillScreen(ILI9341_BLACK);
                    tft.setCursor(10, 50);
                    tft.setTextSize(2);
                    tft.setTextColor(ILI9341_WHITE);
                    tft.printf("Open Compartment %c\n", 'A' + comp);
                    tft.println("to dispense pills.");

                    unsigned long openAttemptStartTime = millis();
                    bool compartmentOpened = false;
                    while (!compartmentOpened && (millis() - openAttemptStartTime < 30000)) { // Wait up to 30 seconds for compartment to open
                        if (isCompartmentOpen(comp)) {
                            compartmentOpened = true;
                            displayMessage("Compartment opened!", ILI9341_GREEN, 1000);
                            break;
                        }
                        delay(100); // Check every 100ms
                    }

                    if (compartmentOpened) {
                        // --- DISPENSING ---
                        compartment[comp].noPillRemaining -= currentSchedule->doseCount;
                        currentSchedule->takenToday = true; // Mark as taken for today
                        delay(5000);// Display warning for 1 seconds
                        digitalWrite(compartmentLEDs[comp], LOW); // Turn off compartment LED

                        displayMessage("Pills dispensed from " + String((char)('A' + comp)) + "!", ILI9341_GREEN, 2000);
                        Serial.println("Pill dispensed");

                        // PUBLISH SUCCESS STATUS TO AWS
                        publishPillStatus(comp, "taken", currentSchedule->doseCount, scheduledTimeForLog);
                        Serial.println("published Pill");
                        
                    } else {
                        digitalWrite(compartmentLEDs[comp], LOW); // Turn off compartment LED
                        displayMessage("Compartment not opened. Pills not dispensed.", ILI9341_ORANGE, 3000);
                        Serial.println("Compartment not opened. Pills not dispensed.");

                         // PUBLISH MISSED STATUS TO AWS
                        publishPillStatus(comp, "missed", currentSchedule->doseCount, scheduledTimeForLog);
                        Serial.println("published Pill");
                    }

                } else {
                    // Not enough pills
                    digitalWrite(BUZZER_PIN, HIGH); // Briefly alert for low pills
                    tft.fillScreen(ILI9341_ORANGE);
                    tft.setCursor(10, 50);
                    tft.setTextSize(2);
                    tft.setTextColor(ILI9341_BLACK);
                    tft.printf("LOW PILLS IN COMPARTMENT %c!\n", 'A' + comp);
                    tft.printf("Need %d, have %d\n", currentSchedule->doseCount, compartment[comp].noPillRemaining);
                    tft.setCursor(10, 180);
                    tft.println("Refill soon!");
                    delay(1000); // Display warning for 1 seconds
                    digitalWrite(BUZZER_PIN, LOW);
                    tft.fillScreen(ILI9341_BLACK); // Clear screen after warning
                }
                // Add a small delay after processing a schedule to prevent multiple triggers in the same minute
                delay(1000);
                tft.fillScreen(ILI9341_BLACK); // Clear screen
            }
        }
    }
}


// ------------------ Display Schedules for a Compartment ------------------
void displayCompartmentSchedules(int compIndex) {
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(10, 10);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.printf("Schedules for Comp %c:\n", 'A' + compIndex);

  int displayY = 40;
  bool schedulesFound = false;
  for (int i = 0; i < MAX_SCHEDULES_PER_COMPARTMENT; i++) {
    MedicineSchedule* sched = &compartment[compIndex].schedule[i];
    if (sched->active) {
      schedulesFound = true;
      String hourStr = (sched->hour < 10 ? "0" : "") + String(sched->hour);
      String minStr = (sched->minute < 10 ? "0" : "") + String(sched->minute);
      tft.setCursor(10, displayY);
      tft.setTextColor(ILI9341_CYAN);
      tft.printf("%d. %s:%s - %d pills ", i + 1, hourStr.c_str(), minStr.c_str(), sched->doseCount);
      tft.setTextColor(sched->active ? ILI9341_GREEN : ILI9341_RED);
      tft.println(sched->active ? "(Active)" : "(Disabled)");
      displayY += 20; // Move to next line
    }
  }

  if (!schedulesFound) {
    tft.setCursor(10, displayY);
    tft.setTextColor(ILI9341_ORANGE);
    tft.println("No schedules set.");
    displayY += 20;
  }

  tft.setCursor(10, displayY + 10);
  tft.setTextColor(ILI9341_WHITE);
  tft.println("Enter schedule # to toggle (or # to exit):");
}

// ------------------ View/Manage Schedules Function ------------------
void viewManageSchedules() {
  char compKey = '\0';
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(10, 10);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("View/Manage Schedules");
  tft.println("Select Compartment A/B/C/D:");

  while ((compKey = keypad.getKey()) == NO_KEY);
  Serial.print("Key Pressed: ");
  Serial.println(compKey);

  int compIndex = -1;
  if (compKey == 'A') compIndex = 0;
  else if (compKey == 'B') compIndex = 1;
  else if (compKey == 'C') compIndex = 2;
  else if (compKey == 'D') compIndex = 3;
  else {
    displayMessage("Invalid Compartment!", ILI9341_RED);
    return;
  }

  bool managing = true;
  while (managing) {
    displayCompartmentSchedules(compIndex); // Show current schedules for selected compartment

    char choiceKey = keypad.getKey();
    if (choiceKey != NO_KEY) {
      if (choiceKey == '#') { // Exit
        managing = false;
        displayMessage("Exiting schedule manager.", ILI9341_WHITE);
      } else if (choiceKey >= '1' && choiceKey <= '6') { // Select a schedule slot
        int schedNum = choiceKey - '1'; // Convert char to 0-5 index
        if (schedNum >= 0 && schedNum < MAX_SCHEDULES_PER_COMPARTMENT) {
          MedicineSchedule* sched = &compartment[compIndex].schedule[schedNum];
          if (sched->active) {
            sched->active = false;
            displayMessage("Schedule " + String(schedNum + 1) + " disabled!", ILI9341_ORANGE);
          } else {
            sched->active = true;
            displayMessage("Schedule " + String(schedNum + 1) + " enabled!", ILI9341_GREEN);
          }
          delay(1000); // Give time to read message
        } else {
          displayMessage("Invalid schedule number!", ILI9341_RED);
        }
      } else {
        displayMessage("Invalid input!", ILI9341_RED);
      }
    }
    delay(100); // Debounce
  }
}

// ------------------ Wi-Fi Connection Function ------------------
void connectWiFi() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(10, 10);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("Connecting to WiFi...");
  tft.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    tft.print(".");
    attempts++;
    if (attempts > 20) { // Try for 10 seconds
      Serial.println("\nWiFi connection failed. Retrying...");
      tft.println("\nWiFi Failed. Retrying...");
      attempts = 0;
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Try to restart connection
    }
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  displayMessage("WiFi Connected!", ILI9341_GREEN, 1500);
}

// ------------------ AWS IoT MQTT Connection Function ------------------
void connectAWS() {
  if (client.connected()) {
    return; // Already connected
  }

  Serial.print("Connecting to AWS IoT...");
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(10, 10);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("Connecting to AWS IoT...");

  // Configure certificates
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_PRIVATE_KEY);

  // Attempt to connect to MQTT broker
  if (client.connect(CLIENT_ID)) {
    Serial.println("Connected to AWS IoT!");
    displayMessage("AWS IoT Connected!", ILI9341_GREEN, 1500);
    client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC); // Subscribe to command topic
    Serial.printf("Subscribed to topic: %s\n", AWS_IOT_SUBSCRIBE_TOPIC);
  } else {
    Serial.print("AWS IoT connection failed, rc=");
    Serial.print(client.state());
    Serial.println(" retrying in 5 seconds");
    displayMessage("AWS IoT Failed! RC:" + String(client.state()), ILI9341_RED, 2000);
    // client.state() returns a value from PubSubClient.h:
    // -4 : MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time
    // -3 : MQTT_CONNECTION_LOST - the network connection was broken
    // -2 : MQTT_CONNECT_FAILED - the network connection failed for some reason
    // -1 : MQTT_DISCONNECTED - the client is disconnected cleanly
    //  0 : MQTT_CONNECTED - the client is connected
    //  1 : MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT
    //  2 : MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier
    //  3 : MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection
    //  4 : MQTT_CONNECT_BAD_CREDENTIALS - the username/password were incorrect
    //  5 : MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect
    delay(5000); // Wait before retrying
  }
}

// ------------------ MQTT Message Callback (for subscribed topics) ------------------
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  Serial.print("Payload: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  Serial.println(messageTemp);

  // Example: Process commands from the cloud
  // if (String(topic) == AWS_IOT_SUBSCRIBE_TOPIC) {
  //   // Parse JSON and act on commands (e.g., update schedule from cloud)
  //   // DynamicJsonDocument doc(256);
  //   // deserializeJson(doc, messageTemp);
  //   // if (doc.containsKey("command") && doc["command"] == "sync_schedule") {
  //   //   // Implement schedule sync logic here
  //   // }
  // }
}

// ------------------ Publish Pill Status to AWS IoT ------------------
void publishPillStatus(int compIndex, String status, int doseCount, DateTime timestamp) {
  Serial.println("In published Pill function");
  if (!client.connected()) {
    connectAWS(); // Reconnect if needed
    if (!client.connected()) {
      Serial.println("Failed to publish: MQTT not connected.");
      return;
    }
  }

  StaticJsonDocument<256> doc;
  doc["device_id"] = "ESP32_PillBox";
  doc["compartment"] = String((char)('A' + compIndex));
  doc["status"] = status;
  doc["doseCount"] = doseCount;
  doc["time"] = timestamp.timestamp(); // Unix timestamp

//  char timeStr[25];
//  sprintf(timeStr, "%04d-%02d-%02dT%02d:%02d:%02d",
//          timestamp.year(), timestamp.month(), timestamp.day(),
//          timestamp.hour(), timestamp.minute(), timestamp.second());
//  doc["time_str"] = timeStr;  // Safe assignment


  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);

  Serial.print("Publishing to ");
  Serial.print(AWS_IOT_PUBLISH_TOPIC);
  Serial.print(": ");
  Serial.println(jsonBuffer);

  if (client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer)) {
    Serial.println("Publish successful");
  } else {
    Serial.println("Publish failed");
  }
}
