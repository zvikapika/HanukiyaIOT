#include "autoconnect_utils.h"
#include "firebase_utils.h"

int master = -1;
int candles = 0;
int mode = 0;
int param1 = 0;
String param2 = "";

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(10);
  Serial.println();
  delay(3000);
  Serial.println("! autoconnecting...");
  autoconnectSetup();
  Serial.println("! setting up firebase...");
  firebaseSetup();
  Serial.println("! ready");
}

void loop() {
  autoconnectUpdate();
  handleSerial();
}

void handleSerial() {
  if (Serial.available()) {
    String pName = "/";
    while(Serial.available() && Serial.peek() != ':') {
      pName += ((char)Serial.read());
      delay(5);
    }
    if(Serial.peek() != ':') {
      while(Serial.available()) { Serial.read(); delay(5); }
      return;
    }
    Serial.read(); // skip ':'
    Serial.println(String("! pName: ") + pName);
    if(pName == "/param2") {
      String pValue = "";
      while(Serial.available() && Serial.peek() != 10 && Serial.peek() != 13) {
        pValue += ((char)Serial.read());
        delay(5);
      }
      Serial.println(String("! pValue(string): ") + pValue);
      firebasePersist(pName, pValue);
    }
    else {
      int pValue = Serial.parseInt();
      Serial.println(String("! pValue: ") + pValue);
      firebasePersist(pName, pValue);
    }
    while(Serial.available()) { Serial.read(); delay(5); }
  }  
}

void onMasterChanged(int newMaster) {
  master = newMaster;
  Serial.print("master:"); Serial.println(master);
}

void onCandlesChanged(int newCandles) {
  candles = newCandles;
  Serial.print("candles:"); Serial.println(candles);
}

void onModeChanged(int newMode) {
  mode = newMode;
  Serial.print("mode:"); Serial.println(mode);
}

void onParam1Changed(int newParam1) {
  param1 = newParam1;
  Serial.print("param1:"); Serial.println(param1);
}

void onParam2Changed(String newParam2) {
  param2 = newParam2;
  Serial.print("param2:"); Serial.print(param2); Serial.println();
}
