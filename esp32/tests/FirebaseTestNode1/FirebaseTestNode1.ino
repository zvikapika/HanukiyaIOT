#include <WiFi.h>
#include "FirebaseESP32.h"

#define FIREBASE_HOST "esp32-samsungx-hanukkah.firebaseio.com"
#define FIREBASE_AUTH "gzTOuyEJi2NKGL1lbutXMMEqlJ9yXC2xdEeexoBO" //Change to your Firebase RTDB secret password
#define WIFI_SSID "sababa"
#define WIFI_PASSWORD "sababababasis"

//Define Firebase Data objects
FirebaseData firebaseData1;
FirebaseData firebaseData2;

bool swState = false;
String path = "/Nodes";
String nodeID = "Node1"; //This is this node ID to receive control
String otherNodeID = "Node2"; //This is other node ID to control

void streamCallback(StreamData data) {

  if (data.dataType() == "boolean") {
    if (data.boolData())
      Serial.println("Set " + nodeID + " to High");
    else
      Serial.println("Set " + nodeID + " to Low");
  }
}

void streamTimeoutCallback(bool timeout) {
  if (timeout) {
    Serial.println();
    Serial.println("Stream timeout, resume streaming...");
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
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  if (!Firebase.beginStream(firebaseData1, path + "/" + nodeID)) {
    Serial.println("Could not begin stream");
    Serial.println("REASON: " + firebaseData1.errorReason());
    Serial.println();
  }
  Firebase.setStreamCallback(firebaseData1, streamCallback, streamTimeoutCallback);
}

void loop() {
  if (Serial.available()) {
    while (Serial.available()) Serial.read();
    bool _swState = swState;
    swState = !swState;
    if (Firebase.setBool(firebaseData2, path + "/" + otherNodeID, swState)) {
      if (swState)
        Serial.println("Set " + otherNodeID + " to High");
      else
        Serial.println("Set " + otherNodeID + " to Low");
    } 
    else {
      swState = _swState;
      Serial.println("Could not set " + otherNodeID);
    }
  }
}
