#include "UsbPins.h"
#include <SPI.h>

#include <Arduino.h>
#include <M5Unified.h>
#include <SD.h>
#include <ArduinoJson.h>
#include "UsbHostProxmark.hpp"
#include "WebUpdate.hpp"
#include "UsbDebugView.hpp"
#include "HexTools.hpp"

UsbHostProxmark pm3;
WebUpdateServer web;
UsbDebugView dbg(pm3);

#include <Update.h>
void performSdUpdate(){
  const char* fw="/firmware.bin";
  if(!SD.begin()){ M5.Display.println("[SD] init failed"); return; }
  if(!SD.exists(fw)){ M5.Display.println("[SD] /firmware.bin not found"); return; }
  File f=SD.open(fw, FILE_READ); if(!f){ M5.Display.println("[SD] open failed"); return; }
  size_t size=f.size(); M5.Display.printf("[SD] flashing %u bytes...\n",(unsigned)size);
  if(!Update.begin(size)){ M5.Display.println("[SD] Update.begin failed"); f.close(); return; }
  uint8_t buf[4096]; size_t n; while((n=f.read(buf,sizeof(buf)))>0){ if(Update.write(buf,n)!=n){ Update.printError(Serial); } }
  f.close(); bool ok=Update.end(true); M5.Display.println(ok?"[SD] OK, reboot...":"[SD] failed"); if(ok){ delay(500); ESP.restart(); }
}

// send helpers
void sendProxmarkAscii(const char* cmd){
  if(!pm3.ready()){ M5.Display.println("[ERR] PM3 not ready"); return; }
  size_t n=strlen(cmd); pm3.write((const uint8_t*)cmd,n); if(n==0||cmd[n-1]!='\n'){ uint8_t nl='\n'; pm3.write(nl); }
  M5.Display.setTextColor(TFT_YELLOW,TFT_BLACK); M5.Display.printf("> %s",cmd); if(n==0||cmd[n-1]!='\n') M5.Display.println();
}
void sendProxmarkRawHex(const String& hex){
  if(!pm3.ready()){ M5.Display.println("[ERR] PM3 not ready"); return; }
  std::vector<uint8_t> p; if(!hexToBytes(hex,p)||p.empty()){ M5.Display.println("[ERR] bad hex"); return; }
  pm3.write(p.data(), p.size()); M5.Display.setTextColor(TFT_YELLOW,TFT_BLACK); M5.Display.print("> RAW "); M5.Display.println(bytesToHexLine(p.data(), p.size()));
}
void pumpProxmarkToDisplay(){
  static char line[256]; static size_t fill=0;
  while(pm3.available()){
    int c=pm3.read(); if(c<0) break;
    char ch=(char)c; if(ch=='\r') continue; if(ch=='\n'){ line[fill]=0; M5.Display.setTextColor(TFT_GREEN,TFT_BLACK); M5.Display.println(line); fill=0; continue; }
    if(isprint((int)ch)){ if(fill<sizeof(line)-1) line[fill++]=ch; } else { if(fill<sizeof(line)-1) line[fill++]='.'; }
    if(fill>=sizeof(line)-1){ line[fill]=0; M5.Display.setTextColor(TFT_GREEN,TFT_BLACK); M5.Display.println(line); fill=0; }
  }
}

// menu
struct MenuItem{ const char* name; void (*action)(); };
void action_hw_tune(){ sendProxmarkAscii("hw tune"); }
void action_hf_search(){ sendProxmarkAscii("hf search"); }
void action_hf_reader(){ sendProxmarkAscii("hf reader"); }
void action_hf_legic_info(){ sendProxmarkAscii("hf legic info"); }
void action_hf_legic_rd(){ sendProxmarkAscii("hf legic rd"); }
void action_hf_legic_dump(){ sendProxmarkAscii("hf legic dump"); }
void action_hf_legic_sniff(){ sendProxmarkAscii("hf legic sniff"); }
void action_hf_mf_info(){ sendProxmarkAscii("hf mf info"); }
void action_hf_14a_info(){ sendProxmarkAscii("hf 14a info"); }
void action_usb_debug(){ dbg.begin(); while(true){ M5.update(); pm3.task(); if(dbg.update()) break; delay(1);} M5.Display.fillScreen(TFT_BLACK); }
void action_sd_update(){ performSdUpdate(); }
void action_ota_hint(){ M5.Display.println("Open / (web) for OTA upload"); }
MenuItem items[] = {
  {"hw tune", action_hw_tune},
  {"hf search", action_hf_search},
  {"hf reader", action_hf_reader},
  {"hf legic info", action_hf_legic_info},
  {"hf legic rd", action_hf_legic_rd},
  {"hf legic dump", action_hf_legic_dump},
  {"hf legic sniff", action_hf_legic_sniff},
  {"hf mf info", action_hf_mf_info},
  {"hf 14a info", action_hf_14a_info},
  {"USB Debug", action_usb_debug},
  {"SD Update", action_sd_update},
  {"OTA (web)", action_ota_hint}
};
int menuCount=sizeof(items)/sizeof(items[0]); int cur=0;
void drawMenu(){
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.fillRect(0,0,320,22,TFT_DARKGREY);
  M5.Display.setTextColor(TFT_WHITE); M5.Display.setCursor(6,6); M5.Display.print("PM3 Host | A=Prev B=Run C=Next");
  int y=34;
  for(int i=0;i<menuCount;i++){ bool s=(i==cur); if(s) M5.Display.fillRect(6,y-2,308,18,M5.Display.color565(30,30,30));
    M5.Display.setCursor(10,y); M5.Display.setTextColor(s?TFT_YELLOW:TFT_GREEN, s?M5.Display.color565(30,30,30):TFT_BLACK);
    M5.Display.print(s?"> ":"  "); M5.Display.print(items[i].name); y+=18; if(y>220) break; }
}

void setup(){
  auto cfg=M5.config(); M5.begin(cfg); M5.Display.setRotation(1);
  drawMenu();
  Serial.begin(115200);
  if(!SD.begin()){ M5.Display.println("[SD] init failed"); }

  web.begin();
  web.server().on("/usb/stats", HTTP_GET, [](AsyncWebServerRequest* req){
   JsonDocument doc(512);
    doc["ready"]=pm3.ready(); doc["bytes_in"]=pm3.bytesIn(); doc["bytes_out"]=pm3.bytesOut(); doc["last_rcv"]=pm3.lastRcvErr(); doc["last_snd"]=pm3.lastSndErr();
    String out; serializeJson(doc,out); req->send(200,"application/json",out);
  });
  web.server().on("/usb/log", HTTP_GET, [](AsyncWebServerRequest* req){
    size_t n=512; if(req->hasParam("n")) n=req->getParam("n")->value().toInt(); String out=pm3.ring().dumpHex(n,true); req->send(200,"text/plain", out);
  });
  web.server().on("/send", HTTP_GET, [](AsyncWebServerRequest* req){
    if(!pm3.ready()){ req->send(503,"text/plain","device not ready"); return; }
    if(!req->hasParam("hex")){ req->send(400,"text/plain","missing ?hex="); return; }
    String hx=req->getParam("hex")->value(); std::vector<uint8_t> p; if(!hexToBytes(hx,p)||p.empty()){ req->send(400,"text/plain","bad hex"); return; }
    size_t sent=pm3.write(p.data(), p.size()); req->send(200,"text/plain", String("sent ")+sent+" bytes");
  });
  web.server().on("/usb/desc", HTTP_GET, [](AsyncWebServerRequest* req){
    if(!pm3.ready()){ req->send(503,"text/plain","device not ready"); return; }
    String t=req->hasParam("type")? req->getParam("type")->value() : "device";
    uint16_t len=req->hasParam("len")? (uint16_t)strtoul(req->getParam("len")->value().c_str(),nullptr,0):256; if(len<9)len=9; if(len>1024)len=1024;
    std::unique_ptr<uint8_t[]> buf(new uint8_t[len]); uint8_t rc=0; uint16_t l=len;
    if(t=="device") rc=pm3.getDeviceDescriptor(buf.get(), &l);
    else if(t=="config") rc=pm3.getConfigDescriptor(buf.get(), &l);
    else if(t=="string"){ uint8_t idx=req->hasParam("index")?(uint8_t)strtoul(req->getParam("index")->value().c_str(),nullptr,0):1; uint16_t lang=req->hasParam("lang")?(uint16_t)strtoul(req->getParam("lang")->value().c_str(),nullptr,0):0x0409; rc=pm3.getStringDescriptor(idx,lang,buf.get(), &l); }
    else if(t=="wcid") rc=pm3.getMsOsDescriptor(buf.get(), &l);
    else { req->send(400,"text/plain","unknown type"); return; }
    if(rc){ req->send(500,"text/plain", String("ERR ")+rc); return; }
    String out; for(uint16_t i=0;i<l;i++){ if(i) out+=' '; char b[4]; sprintf(b,"%02X", buf[i]); out+=b; } req->send(200,"text/plain", out);
  });
  web.server().on("/usb/desc/all", HTTP_GET, [](AsyncWebServerRequest* req){
    if(!pm3.ready()){ req->send(503,"text/plain","device not ready"); return; }
    String out; auto hex=[&](const uint8_t* d,uint16_t n){ String s; for(uint16_t i=0;i<n;i++){ if(i) s+=' '; char b[4]; sprintf(b,"%02X", d[i]); s+=b; } return s; };
    uint8_t rc; uint16_t l; std::unique_ptr<uint8_t[]> b;
    l=18; b.reset(new uint8_t[l]); rc=pm3.getDeviceDescriptor(b.get(), &l); out += "[DEVICE]\n" + (rc? String("ERR ")+rc : hex(b.get(),l)) + "\n\n";
    l=512; b.reset(new uint8_t[l]); rc=pm3.getConfigDescriptor(b.get(), &l); out += "[CONFIG]\n" + (rc? String("ERR ")+rc : hex(b.get(),l)) + "\n\n";
    for(uint8_t idx=1; idx<=3; ++idx){ l=128; b.reset(new uint8_t[l]); rc=pm3.getStringDescriptor(idx,0x0409,b.get(), &l); out += "[STRING "+String(idx)+"]\n" + (rc? String("ERR ")+rc : hex(b.get(),l)) + "\n\n"; }
    l=256; b.reset(new uint8_t[l]); rc=pm3.getMsOsDescriptor(b.get(), &l); out += "[MS_OS 0xEE]\n" + (rc? String("ERR ")+rc : hex(b.get(),l)) + "\n";
    req->send(200,"text/plain", out);
  });

  M5.Display.println("[USB] init...");
  if(!pm3.begin()) M5.Display.println("[USB] init FAILED"); else M5.Display.println("[USB] init OK");
}

void loop(){
  M5.update();
  pm3.task();
  pumpProxmarkToDisplay();
  if(M5.BtnA.wasPressed()){ cur=(cur+menuCount-1)%menuCount; drawMenu(); }
  if(M5.BtnC.wasPressed()){ cur=(cur+1)%menuCount; drawMenu(); }
  if(M5.BtnB.wasPressed()){ items[cur].action(); }
  delay(1);
}

/*** --- SD/USB SPI Init helpers (auto-inserted) --- ***/
static bool g_sd_ok = false;

static void deselect_all_spi_clients() {
  pinMode(SD_CS_CONST, OUTPUT);       digitalWrite(SD_CS_CONST, HIGH);
  pinMode(USB_MAX_CS_CONST, OUTPUT);  digitalWrite(USB_MAX_CS_CONST, HIGH);
}

static void spi_bus_begin_once() {
  static bool done = false;
  if (!done) { SPI.end(); SPI.begin(PM3_SPI_SCK, PM3_SPI_MISO, PM3_SPI_MOSI); done = true; }
}

static bool sd_init_multi() {
  deselect_all_spi_clients();
  spi_bus_begin_once();
  const uint32_t freqs[] = { 25000000UL, 10000000UL, 4000000UL };
  for (uint32_t f : freqs) {
    if (SD.begin(SD_CS_CONST, SPI, f)) { M5_LOGI("[SD] mounted @ %lu Hz", (unsigned long)f); return true; }
    M5_LOGW("[SD] begin(%lu Hz) failed", (unsigned long)f);
    deselect_all_spi_clients();
  }
  return false;
}
/*** --- end helpers --- ***/
