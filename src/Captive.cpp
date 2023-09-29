#include "Captive.h"
#include <WiFi.h>
#include "SPIFFS.h"
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"

namespace Captive
{
  const byte DNS_PORT = 53;
  IPAddress apIP(10, 10, 10, 10); // The default android DNS
  bool _isActive = false;

  AsyncWebServer server(80);

  bool IsActive()
  {
    return _isActive;
  }

  void notFound(AsyncWebServerRequest *request) {
      request->send(404, "text/plain", "Not found");
  }

  void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {

    if (request->method() == HTTP_POST && request->url() == "/networks") {
      File file = SPIFFS.open("/networks", FILE_WRITE);
      file.write(data, len);
      file.close();
      request->send(200, "text/plain", "Saved");
      return;
    }

    if (request->method() == HTTP_POST && request->url() == "/pairCode") {
      File file = SPIFFS.open("/pairCode", FILE_WRITE);
      file.write(data, len);
      file.close();
      request->send(200, "text/plain", "Saved");
      return;
    }

    
    if (request->method() == HTTP_POST && request->url() == "/rmtPin") {
      File file = SPIFFS.open("/rmtPin", FILE_WRITE);
      file.write(data, len);
      file.close();
      request->send(200, "text/plain", "Saved");
      return;
    }

    if (request->method() == HTTP_POST && request->url() == "/estopPin") {
      File file = SPIFFS.open("/estopPin", FILE_WRITE);
      file.write(data, len);
      file.close();
      request->send(200, "text/plain", "Saved");
      return;
    }
  }

  void StopCaptive()
  {
    if (!_isActive) return;

    Serial.println("Stopping captive portal...");
    server.end();
    WiFi.mode(WIFI_STA);
    _isActive = false;
  }

  void Setup() {
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
    server.onRequestBody(handleBody);

    server.on("/reset", HTTP_POST, [] (AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Resetting...");
        ESP.restart();
    });

    server.on("/networks", HTTP_GET, [] (AsyncWebServerRequest *request) {
        File file = SPIFFS.open("/networks", FILE_READ);
        request->send(200, "text/plain", file.readString().c_str());
        file.close();
    });

    server.onNotFound(notFound);
  }

  void StartCaptive()
  {
    if (_isActive)
      return;

    Serial.println("Starting captive portal...");
    _isActive = true;

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP((String("ShockLink-") + WiFi.macAddress()).c_str());
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

    server.begin();

    Serial.println("Server ready.");
  
  }

  void Loop()
  {

  }
}