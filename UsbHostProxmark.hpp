#include <cstdint>
#include <deque>

#ifndef USB_SPI_SCK
#  ifdef PM3_SPI_SCK
#    define USB_SPI_SCK  PM3_SPI_SCK
#    define USB_SPI_MISO PM3_SPI_MISO
#    define USB_SPI_MOSI PM3_SPI_MOSI
#  else
#    define USB_SPI_SCK  18
#    define USB_SPI_MISO 19
#    define USB_SPI_MOSI 23
#  endif
#endif

#ifndef USB_MAX_CS
#define USB_MAX_CS PM3_USB_CS
#endif

#pragma once
#include <Arduino.h>
#include <SPI.h>
#include "Usb.h"
#include "cdcacm.h"
#include "UsbPins.h"
#include "RingLog.hpp"

class UsbHostProxmark : public CDCAsyncOper {
public:
  UsbHostProxmark(): usb_(), acm_(nullptr), ready_(false), bytesIn_(0), bytesOut_(0),
                     lastRcvErr_(0), lastSndErr_(0), dtr_(true), rts_(true) {}
  bool begin(){ SPI.begin(USB_SPI_SCK,USB_SPI_MISO,USB_SPI_MOSI,USB_MAX_CS);
    if(usb_.Init()==-1) return false; acm_=new ACM(&usb_, this); return true; }
  void task(){ usb_.Task(); if(acm_ && acm_->isReady()){ if(!ready_) ready_=true; pullRx_(); } else ready_=false; }
  bool ready() const { return ready_; }
  size_t write(const uint8_t* buf, size_t len){ if(!ready_||!acm_) return 0; uint16_t l=len; uint8_t r=acm_->SndData(l,const_cast<uint8_t*>(buf)); lastSndErr_=r; if(r) return 0; bytesOut_+=l; return l; }
  size_t write(uint8_t b){ return write(&b,1); }
  int available() const { return (int)rx_.size(); }
  int read(){ if(rx_.empty()) return -1; uint8_t v=rx_.front(); rx_.pop_front(); return v; }
  uint32_t bytesIn() const { return bytesIn_; } uint32_t bytesOut() const { return bytesOut_; }
  uint8_t lastRcvErr() const { return lastRcvErr_; } uint8_t lastSndErr() const { return lastSndErr_; }
  void setDTR(bool v){ dtr_=v; applyCtrl_(); } void setRTS(bool v){ rts_=v; applyCtrl_(); }
  bool getDTR() const { return dtr_; } bool getRTS() const { return rts_; }
  void toggleDTR(){ setDTR(!dtr_); } void toggleRTS(){ setRTS(!rts_); }
  void setLineCoding(uint32_t baud,uint8_t sb,uint8_t par,uint8_t db){ if(!acm_) return; LINE_CODING lc={baud,sb,par,db}; acm_->SetLineCoding(&lc); lc_=lc; }
  LINE_CODING getLineCoding() const { return lc_; }
  uint8_t OnInit(ACM* pacm) override { LINE_CODING lc={115200,0,0,8}; pacm->SetLineCoding(&lc); lc_=lc; pacm->SetControlLineState((dtr_?1:0)|(rts_?2:0)); return 0; }
  void softResetBus(){ usb_.Init(); }
private:
  USB usb_;
  ACM* acm_;
  bool ready_;
  uint32_t bytesIn_, bytesOut_;
  uint8_t lastRcvErr_, lastSndErr_;
  bool dtr_, rts_;
  LINE_CODING lc_;
  std::deque<uint8_t> rx_;
  void pullRx_(){ uint8_t buf[64]; uint16_t l=sizeof(buf); uint8_t r=acm_->RcvData(&l,buf); lastRcvErr_=r; if(!r&&l){ bytesIn_+=l; rx_.insert(rx_.end(),buf,buf+l); } }
  void applyCtrl_(){ if(!acm_) return; acm_->SetControlLineState((dtr_?1:0)|(rts_?2:0)); }
};
