#include <WiFi.h>

#define USE_WIFI

//#define WIFI_SSID "Keren1978"
//#define WIFI_PASSWORD "Keren1978"
#define WIFI_SSID "sababa"
#define WIFI_PASSWORD "sababababasis"

// ESP32 analog GPIO pins: 2, 4, 12, 13, 14, 15, 25, 26, 27, 32, 33, 34, 35, 36(SVP), 39(SVN)

// Input 1: SVP (GPIO 36)
// Input 2: SVN (GPIO 39)
// Input 3: GPIO 34
// Input 4: GPIO 35
// Input 5: GPIO 32
// Input 6: GPIO 33
// Input 7: GPIO 25
// Input 8: GPIO 26

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
  }
  Serial.println(analogRead(pin)); 
}
