void setup()
{
  Serial.begin(115200);
  delay(1000); // give me time to bring up serial monitor
  Serial.println("ESP32 Touch Test");
}

void loop()
{
  Serial.print(touchRead(T6));  // get value using T0
  Serial.print("\t");
  Serial.println(touchRead(T7));  // get value using T0
//  delay(1000);
}
