#include "autoconnect_utils.h"
#include "firebase_utils.h"

int master = -1;
int candles = 0;

void setup() {
  Serial.begin(115200);
  Serial.println();
  autoconnectSetup();
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  firebaseSetup();
}

void loop() {
  if (Serial.available()) {
    while (Serial.available()) Serial.read();
    int newCandles = random(0, 255);
    if (Firebase.setInt(firebaseData2, path + "/candles" , newCandles)) {
      Serial.println(String("Candles set to ") + newCandles);
    }
    else {
      Serial.println("Could not set new master");
    }
  }
}


void onNewMaster(int newMaster) {
  master = newMaster;
  Serial.print("master set to: "); Serial.println(master);
}


void onNewCandlesState(int newCandles) {
  candles = newCandles;
  Serial.print("candles set to: "); Serial.println(candles);
}
