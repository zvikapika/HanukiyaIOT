#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
typedef WebServer WEBServer;
#include <FS.h>
#include <AutoConnect.h>

#define HELLO_URI   "/hello"
#define PARAM_STYLE "/style.json"

WebServer Server;
AutoConnectConfig Config;       // Enable autoReconnect supported on v0.9.4

ACText(Caption, "Hello, world");

//AutoConnectAux for the custom Web page.
AutoConnectAux helloPage(HELLO_URI, "Hello", true, { Caption });
AutoConnect Portal(Server);

// JSON document loading buffer
String ElementJson;

void onRoot() {
  WEBServer& webServer = Portal.host();
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
  SPIFFS.begin(true); // true means format if unreadable
  File param = SPIFFS.open(fileName, "r");
  if (param) {
    ElementJson = param.readString();
    param.close();
  }
  Portal.config(Config);
  SPIFFS.end();
}

void autoconnectSetup() {
  loadParam(PARAM_STYLE);     // Pre-load the element from JSON.
  helloPage.on(onHello);      // Register the attribute overwrite handler.
  Portal.join(helloPage);     // Join the hello page.
  Portal.begin();

  WEBServer& webServer = Portal.host();
  webServer.on("/", onRoot);  // Register the root page redirector.

//  Config.autoReconnect = true;
  Config.autoReconnect = false;
  Config.portalTimeout = 60000L;
  Config.apid = "XHanukkah";
  Config.psk  = "12345678";
  Config.retainPortal = true;

  Portal.config(Config);

}

void autoconnectUpdate() {
  Portal.handleClient();
}
