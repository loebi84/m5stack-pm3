
#pragma once
#include <M5Unified.h>
#include "UsbHostProxmark.hpp"
#include "HexTools.hpp"
class UsbDebugView{
  UsbHostProxmark& host_; int sel_=0; static constexpr int N=8;
public:
  explicit UsbDebugView(UsbHostProxmark& h):host_(h){}
  void begin(){ M5.Display.fillScreen(TFT_BLACK); M5.Display.fillRect(0,0,320,22,TFT_DARKGREY); M5.Display.setTextColor(TFT_WHITE); M5.Display.setCursor(6,6); M5.Display.print("USB Debug (B=Run  B-hold=Back)"); draw_(); }
  bool update(){ if(M5.BtnA.wasPressed()){ sel_=(sel_+N-1)%N; draw_(); } if(M5.BtnC.wasPressed()){ sel_=(sel_+1)%N; draw_(); } if(M5.BtnB.wasPressed()){ run_(); draw_(); } return M5.BtnB.pressedFor(900); }
private:
  void draw_(){ M5.Display.fillRect(4,24,312,212,TFT_BLACK); M5.Display.setCursor(8,28); M5.Display.setTextColor(TFT_CYAN); M5.Display.printf("ready:%d in:%lu out:%lu rc:%u sc:%u", host_.ready(), (unsigned long)host_.bytesIn(), (unsigned long)host_.bytesOut(), host_.lastRcvErr(), host_.lastSndErr()); const char* it[N]={"Soft reset","Toggle DTR","Toggle RTS","LC 9600","LC 115200","Snap 256","Snap 1024","Dump descriptors"}; int y=46; for(int i=0;i<N;i++){ bool s=i==sel_; if(s) M5.Display.fillRect(6,y-2,308,14,M5.Display.color565(30,30,30)); M5.Display.setCursor(10,y); M5.Display.setTextColor(s?TFT_YELLOW:TFT_GREEN, s?M5.Display.color565(30,30,30):TFT_BLACK); M5.Display.printf("%c %s", s?'>':' ', it[i]); y+=14; } }
  void run_(){ switch(sel_){ case 0: host_.softResetBus(); toast_("soft reset"); break; case 1: host_.toggleDTR(); toast_(String("DTR=")+(host_.getDTR()?"1":"0")); break; case 2: host_.toggleRTS(); toast_(String("RTS=")+(host_.getRTS()?"1":"0")); break; case 3: host_.setLineCoding(9600,0,0,8); toast_("LC=9600"); break; case 4: host_.setLineCoding(115200,0,0,8); toast_("LC=115200"); break; case 5: snap_(256); break; case 6: snap_(1024); break; case 7: dump_(); break; } }
  void toast_(const String& s){ M5.Display.fillRect(8,120,304,16,TFT_BLACK); M5.Display.setTextColor(TFT_YELLOW); M5.Display.setCursor(8,120); M5.Display.print(s); }
  void snap_(size_t n){ auto hex=host_.ring().dumpHex(n,true); M5.Display.fillRect(8,140,304,72,TFT_BLACK); M5.Display.setCursor(8,140); M5.Display.setTextColor(TFT_WHITE); size_t col=0; for(size_t i=0;i<hex.length();++i){ char c=hex[i]; M5.Display.print(c); if(++col>46 && c==' '){ M5.Display.println(); col=0; } } }
  void dump_(){ uint8_t b[256]; uint16_t l; uint8_t rc; M5.Display.fillRect(8,140,304,72,TFT_BLACK); M5.Display.setCursor(8,140); M5.Display.setTextColor(TFT_WHITE);
    auto phex=[&](const uint8_t* d,uint16_t n){ for(uint16_t i=0;i<n;i++){ char x[4]; sprintf(x,"%02X",d[i]); M5.Display.print(x); if(i<n-1) M5.Display.print(' '); if(i%24==23) M5.Display.println(); } M5.Display.println(); };
    l=18; rc=host_.getDeviceDescriptor(b,&l); M5.Display.print("[DEV] "); if(rc){ M5.Display.printf("ERR %u\n",rc);} else phex(b,l);
    l=256; rc=host_.getConfigDescriptor(b,&l); M5.Display.print("[CFG] "); if(rc){ M5.Display.printf("ERR %u\n",rc);} else phex(b,(l>96?96:l));
    for(uint8_t idx=1; idx<=3; ++idx){ l=64; rc=host_.getStringDescriptor(idx,0x0409,b,&l); M5.Display.printf("[STR %u] ",idx); if(rc){ M5.Display.printf("ERR %u\n",rc);} else phex(b,l); }
    l=128; rc=host_.getMsOsDescriptor(b,&l); M5.Display.print("[MS 0xEE] "); if(rc){ M5.Display.printf("ERR %u\n",rc);} else phex(b,l);
  }
};
