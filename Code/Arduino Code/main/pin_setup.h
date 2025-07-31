
// ------------------ TFT Setup ------------------
#define TFT_CS   5
#define TFT_DC   27
#define TFT_RST  33
#define TFT_MOSI 23
#define TFT_SCK  18
#define TFT_LED  22

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

//------------------- Buzzer ------------------------
#define BUZZER_PIN 4 // Example pin for a buzzer/alarm

//------------------- led ---------------------------
// Define LED pins for each compartment (adjust these pins as per your wiring)
#define COMP_A_LED_PIN 16 // Example LED pin for Compartment A
#define COMP_B_LED_PIN 17 // Example LED pin for Compartment B
#define COMP_C_LED_PIN 2 // Example LED pin for Compartment C
#define COMP_D_LED_PIN 19 // Example LED pin for Compartment D 

// Array to hold LED pins for easy access - MOVED TO TOP FOR SCOPE RESOLUTION
const int compartmentLEDs[4] = {COMP_A_LED_PIN, COMP_B_LED_PIN, COMP_C_LED_PIN, COMP_D_LED_PIN};

//------------------- IR sensor ---------------------
// Define IR Sensor pins for each compartment (adjust these pins as per your wiring)
// Assuming IR sensor outputs LOW when open/detected, HIGH when closed/not detected
#define COMP_A_IR_PIN 34 // Example IR pin for Compartment A (Input only pin)
#define COMP_B_IR_PIN 35 // Example IR pin for Compartment B (Input only pin)
#define COMP_C_IR_PIN 36 // Example IR pin for Compartment C (Input only pin)
#define COMP_D_IR_PIN 39 // Example IR pin for Compartment D (Input only pin)

// Array to hold IR sensor pins for easy access - MOVED TO TOP FOR SCOPE RESOLUTION
const int compartmentIRs[4] = {COMP_A_IR_PIN, COMP_B_IR_PIN, COMP_C_IR_PIN, COMP_D_IR_PIN};

// AWS IoT Client Setup
WiFiClientSecure net;
PubSubClient client(net);
long lastReconnectAttempt = 0; // For MQTT connection management
