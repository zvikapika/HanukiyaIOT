#include <WiFi.h>

//#define USE_WIFI

#define WIFI_SSID "Keren1978"
#define WIFI_PASSWORD "Keren1978"
//#define WIFI_SSID "sababa"
//#define WIFI_PASSWORD "sababababasis"

void setup() {
  Serial.begin(115200);
#ifdef USE_WIFI
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
#endif
}

int pin = 2;

void loop() {
  if (Serial.available()) {
    pin = Serial.parseInt();
    pinMode(pin, INPUT);
  }
  Serial.println(digitalRead(pin)); 
  delay(10);
}
