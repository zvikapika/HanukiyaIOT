byte ANALOG_INPUT_PINS[] = {A0, A1, A2, A3, A4, A5, A6, A7};

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
