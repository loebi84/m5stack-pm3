
#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include <SD.h>
class WebUpdateServer{
  AsyncWebServer server_;
public:
  explicit WebUpdateServer(uint16_t port=80): server_(port) {}
  void begin(){
    server_.on("/", HTTP_GET, [&](AsyncWebServerRequest* req){
      req->send(200,"text/html",
        "<html><body><h3>M5 PM3 Host</h3>"
        "<form method='POST' action='/update' enctype='multipart/form-data'>"
        "<input type='file' name='firmware'><input type='submit' value='Upload & Flash'></form>"
        "<p><a href='/sdupdate'>Flash /sd/firmware.bin</a></p>"
        "<p><a href='/usb/stats'>USB Stats</a> | <a href='/usb/log'>USB Log</a> | <a href='/usb/desc/all'>Descriptors</a></p>"
        "<form method='GET' action='/send'><input name='hex' style='width:90%'><input type='submit' value='Send RAW'></form>"
        "</body></html>");
    });
    server_.on("/update", HTTP_POST,
      [&](AsyncWebServerRequest* request){
        bool ok=!Update.hasError();
        request->send(200,"text/plain", ok?"OK. Rebooting...":"Update failed");
        if(ok){ delay(300); ESP.restart(); }
      },
      [&](AsyncWebServerRequest* req, String filename, size_t index, uint8_t* data, size_t len, bool final){
        if(!index){ Update.begin(); }
        if(Update.write(data,len)!=len){ Update.printError(Serial); }
        if(final){ if(!Update.end(true)) Update.printError(Serial); }
      });
    server_.on("/sdupdate", HTTP_GET, [&](AsyncWebServerRequest* req){
      req->send(200,"text/plain","Use on-device menu to flash from SD.");
    });
    server_.on("/usb/stats", HTTP_GET, [&](AsyncWebServerRequest* r){ r->send(200,"application/json","{}"); });
    server_.on("/usb/log", HTTP_GET, [&](AsyncWebServerRequest* r){ r->send(200,"text/plain",""); });
    server_.on("/usb/desc", HTTP_GET, [&](AsyncWebServerRequest* r){ r->send(200,"text/plain",""); });
    server_.on("/usb/desc/all", HTTP_GET, [&](AsyncWebServerRequest* r){ r->send(200,"text/plain",""); });
    server_.on("/send", HTTP_GET, [&](AsyncWebServerRequest* r){ r->send(200,"text/plain","device not ready"); });
    server_.begin();
  }
  AsyncWebServer& server(){ return server_; }
};
