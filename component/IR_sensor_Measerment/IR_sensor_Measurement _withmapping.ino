#define IR_SENSOR_PIN 4  // Analog-capable GPIO (double-check for your hardware)

void setup() {
  Serial.begin(115200);
  pinMode(IR_SENSOR_PIN, INPUT);
}

// Floating-point mapping function
float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void loop() {
  int sensorValue = analogRead(IR_SENSOR_PIN);

  // Map raw sensor value to distance in cm (adjust based on your sensor)
  float distance_cm = mapf(sensorValue, 100, 3000, 80, 10); 
  float distance_m = distance_cm / 100.0;

  // Only print 0 (open) or 1 (closed)
  if (distance_m < 0.2) {
    Serial.println("0"); // Box OPEN
  } else {
    Serial.println("1"); // Box CLOSED
  }

  delay(200);
}
