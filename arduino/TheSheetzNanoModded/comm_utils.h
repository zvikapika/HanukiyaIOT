#include <SoftwareSerial.h>

SoftwareSerial wifi(9, 10);
String tmpBuffer = "";

void onNewMaster(int newMaster);
void onNewCandlesState(int newCandles);
void onNewMode(int newMode);
void onNewParam1(int newParam1);
void onNewParam2(String newParam2);

void commSetup() {
  wifi.begin(9600);
}

void handleIncomingLine(String line) {
  Serial.println(String("<-- COMM: '") + line + "'");
  if (!line.startsWith("!")) {
    int colonIndex = line.indexOf(":");
    if (colonIndex > -1) {
      String pName = line.substring(0, colonIndex);
      String pValue = line.substring(colonIndex + 1);
      Serial.print("got remote parameter set: ");
      Serial.print(pName);
      Serial.print("=");
      Serial.println(pValue);
      if (remoteEnabled) {
        if (pName == "master") {
          onNewMaster(pValue.toInt());
        }
        else if (pName == "candles") {
          onNewCandlesState(pValue.toInt());
        }
        else if (pName == "mode") {
          onNewMode(pValue.toInt());
        }
        else if (pName == "param1") {
          onNewParam1(pValue.toInt());
        }
        else if (pName == "param2") {
          onNewParam2(pValue);
        }
      }
      else {
        Serial.println("parameter not set, remote mode disabled");
      }
    }
    else {
      Serial.print("ERROR: could not parse line: '");
      Serial.print(line);
      Serial.println("'");
    }
  }
  else {
    //    Serial.print("comment, discarding");
  }
}

void handleComm() {
  if (wifi.available()) {
    char c = wifi.read();
    if (c == 13) {
      delay(10);

      tmpBuffer.trim();
      handleIncomingLine(tmpBuffer);
      tmpBuffer = "";
    }
    else {
      tmpBuffer += c;
    }
  }
}

void updateCandles(int val) {
  if (val >= 0 && val <= 255) {
    Serial.print("UPDATING CANDLES TO: "); Serial.println(val);
    wifi.print(String("candles:") + val);
    wifi.println();
  }
  else {
    Serial.println(String("illegal candles value: ") + val);
  }
}

void updateMode(int val) {
  if (val >= 0 && val <= SPECIAL_MODE_COUNT) {
    Serial.print("UPDATING MODE TO: "); Serial.println(val);
    wifi.print(String("mode:") + val);
    wifi.println();
  }
  else {
    Serial.println(String("illegal candles value: ") + val);
  }
}

void updateParam1(int val) {
  Serial.print("UPDATING PARAM2 TO: "); Serial.println(val);
  wifi.print(String("param1:") + val);
  wifi.println();
}

void updateParam2(String val) {
  Serial.print("UPDATING PARAM2 TO: "); Serial.print(val);
  wifi.print(String("param2:") + val);
  wifi.println();
}
