#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
typedef ESP8266WebServer WEBServer;
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
typedef WebServer WEBServer;
#endif
#include <FS.h>
#include <AutoConnect.h>

#define HELLO_URI   "/hello"
#define PARAM_STYLE "/style.json"

//WebServer Server;
AutoConnectConfig Config;       // Enable autoReconnect supported on v0.9.4

ACText(Caption, "Hello, world");

//AutoConnectAux for the custom Web page.
AutoConnectAux helloPage(HELLO_URI, "Hello", true, { Caption });
//AutoConnect Portal(Server);
AutoConnect portal;


// JSON document loading buffer
String ElementJson;

void onRoot() {
  WEBServer& webServer = portal.host();
  webServer.sendHeader("Location", String("http://") + webServer.client().localIP().toString() + String(HELLO_URI));
  webServer.send(302, "text/plain", "");
  webServer.client().flush();
  webServer.client().stop();
}

// Load the attribute of the element to modify at runtime from external.
String onHello(AutoConnectAux& aux, PageArgument& args) {
  aux.loadElement(ElementJson);
  return String();
}

// Load the element from specified file in SPIFFS.
void loadParam(const char* fileName) {
  SPIFFS.begin(); // true means format if unreadable
  File param = SPIFFS.open(fileName, "r");
  if (param) {
    ElementJson = param.readString();
    param.close();
  }
  portal.config(Config);
  SPIFFS.end();
}

void autoconnectSetup() {
  loadParam(PARAM_STYLE);     // Pre-load the element from JSON.
  helloPage.on(onHello);      // Register the attribute overwrite handler.
  portal.join(helloPage);     // Join the hello page.
  portal.begin();

  WEBServer& webServer = portal.host();
  webServer.on("/", onRoot);  // Register the root page redirector.

  Config.autoReconnect = true;
  Config.portalTimeout = 60000L;
  Config.apid = "XHanukkah";
  Config.psk  = "12345678";
//  Config.retainPortal = true;
  Config.retainPortal = false;
  portal.config(Config);
  Serial.print("! Connected with IP: ");
  Serial.println(WiFi.localIP());
}

void autoconnectUpdate() {
  portal.handleClient();
}
