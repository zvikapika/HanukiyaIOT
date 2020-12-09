#include <SoftwareSerial.h>

SoftwareSerial wifi(8,9);

void setup() {
  Serial.begin(9600);
  wifi.begin(9600);

}

void loop() {
  if(Serial.available()) wifi.write(Serial.read());
  if(wifi.available()) Serial.write(wifi.read());
}
