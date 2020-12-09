#if defined(ARDUINO_ARCH_ESP8266)
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include "FirebaseESP32.h"
#endif

#define FIREBASE_HOST "esp32-samsungx-hanukkah.firebaseio.com"
#define FIREBASE_AUTH "gzTOuyEJi2NKGL1lbutXMMEqlJ9yXC2xdEeexoBO" //Change to your Firebase RTDB secret password

FirebaseData firebaseData;
FirebaseData firebaseData2;

String path = "/State";

void handleNullStreamCallback();
void handleJSONStreamCallback(StreamData data);
void handleIntStreamCallback(StreamData data);
void handleStringStreamCallback(StreamData data);

void onMasterChanged(int newMaster);
void onCandlesChanged(int newCandles);
void onModeChanged(int newMode);
void onParam1Changed(int newParam1);
void onParam2Changed(String newParam2);

#include "printResult.h"

void streamCallback(StreamData data) {
  String dataType = data.dataType();
  Serial.print("! got data: ");
  Serial.println(dataType);

  if (dataType == "null") {
    handleNullStreamCallback();
  }
  else if (dataType == "json") {
    handleJSONStreamCallback(data);
  }
  else if (dataType == "int") {
    handleIntStreamCallback(data);
  }
  else if (dataType == "string") {
    handleStringStreamCallback(data);
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
    onMasterChanged(jsonData.intValue);
  }
  else {
    Serial.print("unexpected data type:"); Serial.println(jsonData.type);
  }
  // extract new candles state
  json->get(jsonData, "candles");
  if (jsonData.type == "int") {
    onCandlesChanged(jsonData.intValue);
  }
  else {
    Serial.print("unexpected data type:"); Serial.println(jsonData.type);
  }
  json->get(jsonData, "mode");
  if (jsonData.type == "int") {
    onModeChanged(jsonData.intValue);
  }
  else {
    Serial.print(String("! unexpected mode data type:") + jsonData.type);
  }
  json->get(jsonData, "param1");
  if (jsonData.type == "int") {
    onParam1Changed(jsonData.intValue);
  }
  else {
    Serial.print(String("! unexpected param1 data type:") + jsonData.type);
  }
  json->get(jsonData, "param2");
  if (jsonData.type == "string") {
    onParam2Changed(jsonData.stringValue);
  }
  else {
    Serial.print(String("! unexpected param2 data type:") + jsonData.type);
  }
}

void handleIntStreamCallback(StreamData data) {
  String dataPath = data.dataPath();
  String dataType = data.dataType();
  Serial.println(String("! path: ") + dataPath + ", type:" +  dataType);
  if (dataPath == "/master" && dataType == "int") {
    int dataValue = data.intData();
    onMasterChanged(dataValue);
  }
  else if (dataPath == "/candles" && dataType == "int") {
    int dataValue = data.intData();
    onCandlesChanged(dataValue);
  }
  else if (dataPath == "/mode" && dataType == "int") {
    int dataValue = data.intData();
    onModeChanged(dataValue);
  }
  else if (dataPath == "/param1" && dataType == "int") {
    int dataValue = data.intData();
    onParam1Changed(dataValue);
  }
  else {
    Serial.print("! unexpected incoming data path: "); Serial.println(dataPath);
  }
}

void handleStringStreamCallback(StreamData data) {
  String dataPath = data.dataPath();
  String dataType = data.dataType();
  Serial.println(String("! path: ") + dataPath + ", type:" +  dataType);
  if (dataPath == "/param2" && dataType == "string")  {
    String dataValue = data.stringData();
    onParam2Changed(dataValue);
  }
  else {
    Serial.print("! unexpected incoming data path: "); Serial.println(dataPath);
  }
}
void streamTimeoutCallback(bool timeout) {
  if (timeout) {
    Serial.println("! Stream timeout :(");
  }
}

void processJSONData(FirebaseData &data) {
  Serial.print("! "); Serial.println(data.payload());
  if (data.dataType() == "json") {
    FirebaseJson &json = data.jsonObject();
    size_t len = json.iteratorBegin();
    String key, value = "";
    int type = 0;
    for (size_t i = 0; i < len; i++)
    {
      json.iteratorGet(i, type, key, value);
      if (type == FirebaseJson::JSON_OBJECT) {
        if (key == "master") onMasterChanged(value.toInt());
        else if (key == "candles") onCandlesChanged(value.toInt());
        else if (key == "mode")    onModeChanged(value.toInt());
        else if (key == "param1")  onParam1Changed(value.toInt());
        else if (key == "param2")  onParam2Changed(value);
      }
    }
    json.iteratorEnd();
  }
  else {
    Serial.println("! error: no json data");
  }
}


void firebaseSetup() {
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  if (!Firebase.beginStream(firebaseData, path)) {
    Serial.print("! Could not begin stream, ");
    Serial.println("reason : " + firebaseData.errorReason());
  }
  Firebase.setStreamCallback(firebaseData, streamCallback, streamTimeoutCallback);
  //  FirebaseJson* json;
  delay(1000);
  if (!Firebase.getJSON(firebaseData, path)) {
    Serial.print("! Could not get master JSON, ");
    Serial.println("reason : " + firebaseData.errorReason());
  }
  else {
    processJSONData(firebaseData);
  }
}

void firebasePersist(String subpath, int value) {
  if (Firebase.setInt(firebaseData2, path + subpath , value)) {
    Serial.println(String("! ok, ") + subpath + " set to: " + value);
  }
  else {
    Serial.println(String("! could not set ") + subpath);
  }
}

void firebasePersist(String subpath, String value) {
  if (Firebase.setString(firebaseData2, path + subpath , value)) {
    Serial.println(String("! ok, ") + subpath + " set to: " + value);
  }
  else {
    Serial.println(String("! could not set ") + subpath);
  }
}
