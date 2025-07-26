#define IR_SENSOR_PIN 4  // Analog-capable GPIO

void setup() {
  Serial.begin(115200);
  pinMode(IR_SENSOR_PIN, INPUT);
}

void loop() {
  int sensorValue = analogRead(IR_SENSOR_PIN);

  // Example: Map sensor values 100 (far) to 3000 (near) to 0.8m to 0.1m
  // Adjust mapping values based on your sensor's real behavior!
  float distance_cm = map(sensorValue, 100, 3000, 80, 10); // distance in cm
  float distance_m = distance_cm / 100.0; // convert to meters

  Serial.print("Sensor Value: ");
  Serial.println(sensorValue);

  // If distance < 0.2m (20cm), display 0 meter, else display the actual distance in meters
  if (distance_m < 0.2) {
    Serial.println("Distance: 0 meter");
  } else {
    Serial.print("Distance: ");
    Serial.print(distance_m, 2); // show 2 decimal points
    Serial.println(" meter");
  }

  delay(200);
}
