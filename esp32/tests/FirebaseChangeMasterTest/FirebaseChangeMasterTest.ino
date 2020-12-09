#include <WiFi.h>
#include "FirebaseESP32.h"

#define FIREBASE_HOST "esp32-samsungx-hanukkah.firebaseio.com"
#define FIREBASE_AUTH "gzTOuyEJi2NKGL1lbutXMMEqlJ9yXC2xdEeexoBO" //Change to your Firebase RTDB secret password
#define WIFI_SSID "Keren1978"
#define WIFI_PASSWORD "Keren1978"

//Define Firebase Data objects
FirebaseData firebaseData;
FirebaseData firebaseData2;

String path = "/State";

void streamCallback(StreamData data) {
  Serial.print("got data: ");
  Serial.println(data.stringData());
//
//  if (data.dataType() == "boolean") {
//    if (data.boolData())
//      Serial.println("Set " + nodeID + " to High");
//    else
//      Serial.println("Set " + nodeID + " to Low");
//  }
}

void streamTimeoutCallback(bool timeout) {
  if (timeout) {
    Serial.println();
    Serial.println("Stream timeout :(");
    Serial.println();
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  if (!Firebase.beginStream(firebaseData, path/* + "/" + nodeID */)) {
    Serial.println("Could not begin stream");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println();
  }
  Firebase.setStreamCallback(firebaseData, streamCallback, streamTimeoutCallback);
}

void loop() {
  if (Serial.available()) {
    while (Serial.available()) Serial.read();
    int newMaster = random(1,100);
    if (Firebase.setInt(firebaseData2, path + "/master" , newMaster)) {
        Serial.println(String("Master set to ") + newMaster);
    } 
    else {
      Serial.println("Could not set new master");
    }
    int newCandles = random(0,255);
    if (Firebase.setInt(firebaseData2, path + "/candles" , newCandles)) {
        Serial.println(String("candles set to ") + newCandles);
    } 
    else {
      Serial.println("could not set new candles");
    }
  }
}
