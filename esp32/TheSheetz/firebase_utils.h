#include <WiFi.h>
#include "FirebaseESP32.h"

#define FIREBASE_HOST "esp32-samsungx-hanukkah.firebaseio.com"
#define FIREBASE_AUTH "gzTOuyEJi2NKGL1lbutXMMEqlJ9yXC2xdEeexoBO" //Change to your Firebase RTDB secret password

FirebaseData firebaseData;
FirebaseData firebaseData2;

String path = "/State";

void handleNullStreamCallback();
void handleJSONStreamCallback(StreamData data);
void handleIntStreamCallback(StreamData data);

void onNewMaster(int master);
void onNewCandlesState(int candles);

void streamCallback(StreamData data) {
  String dataType = data.dataType();
  Serial.print("got data: ");
  Serial.println(dataType);

  if (dataType == "null") {
    handleNullStreamCallback();
  }
  else if (dataType == "json") {
    handleJSONStreamCallback(data);
  }
  else if (dataType == "int") {
    heap_caps_check_integrity_all(true);
    handleIntStreamCallback(data);
  }
}

void handleNullStreamCallback() {
  Serial.println("got null data type, dismissing firebase stream callback");
}

void handleJSONStreamCallback(StreamData data) {
  FirebaseJson* json = data.jsonObjectPtr();
  FirebaseJsonData jsonData;
  // extract new master id
  json->get(jsonData, "master");
  if (jsonData.type == "int") {
    onNewMaster(jsonData.intValue);
  }
  else {
    Serial.print("unexpected data type:"); Serial.println(jsonData.type);
  }
  // extract new candles state
  json->get(jsonData, "candles");
  if (jsonData.type == "int") {
    onNewCandlesState(jsonData.intValue);
  }
  else {
    Serial.print("unexpected data type:"); Serial.println(jsonData.type);
  }
}

void handleIntStreamCallback(StreamData data) {
  String dataPath = data.dataPath();
  int dataValue = data.intData();
  if (dataPath == "/master") {
    onNewMaster(dataValue);
  }
  else if (dataPath == "/candles") {
    onNewCandlesState(dataValue);
  }
  else {
    Serial.print("unexpected incoming data path: "); Serial.println(dataPath);
  }
}

void streamTimeoutCallback(bool timeout) {
  if (timeout) {
    Serial.println();
    Serial.println("Stream timeout :(");
    Serial.println();
  }
}

void firebaseSetup() {
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  if (!Firebase.beginStream(firebaseData, path/* + "/" + nodeID */)) {
    Serial.println("Could not begin stream");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println();
  }
  Firebase.setStreamCallback(firebaseData, streamCallback, streamTimeoutCallback);
}

void firebasePersist(int newCandlesState) {
  if (Firebase.setInt(firebaseData2, path + "/candles" , newCandlesState)) {
    Serial.println(String("Candles set to ") + newCandlesState);
  }
  else {
    Serial.println("Could not set new master");
  }
}
