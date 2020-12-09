
// ESP32 analog GPIO pins: 2, 4, 12, 13, 14, 15, 25, 26, 27, 32, 33, 34, 35, 36(SVP), 39(SVN)

// Input 1: SVP (GPIO 36)
// Input 2: SVN (GPIO 39)
// Input 3: GPIO 34
// Input 4: GPIO 35
// Input 5: GPIO 32
// Input 6: GPIO 33
// Input 7: GPIO 25
// Input 8: GPIO 26

//byte ANALOG_INPUT_PINS[] = {36, 39, 34, 35, 32, 33, 25, 26};
//byte ANALOG_INPUT_PINS[] = {36, 39, 34, 35, 32, 33, 13, 12};
byte ANALOG_INPUT_PINS[] = {36, 39, 34, 35, 32, 33, 4, 2};

void setup() {
  Serial.begin(115200);
}

void loop() {
  for(int i = 0; i < sizeof(ANALOG_INPUT_PINS); ++i) {
    int sensorValue = analogRead(ANALOG_INPUT_PINS[i]);
    Serial.print(sensorValue);
    Serial.print('\t');
  }
  Serial.println();
  delay(1);
}
