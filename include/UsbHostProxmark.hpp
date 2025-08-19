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
  RingLog& ring(){ return ring_; }
  uint8_t devAddr() const { return acm_? acm_->GetAddress():0; }
  uint8_t getDescriptor(uint8_t type,uint8_t index,uint16_t langid,uint8_t* buf,uint16_t* plen){
    if(!acm_||!ready_) return 0xFF; uint8_t addr=devAddr(); if(!addr) return 0xFE;
    const uint8_t bmReqType=0x80, bRequest=0x06; uint8_t wValLo=index, wValHi=type;
    uint8_t wIndLo=(uint8_t)(langid&0xFF), wIndHi=(uint8_t)(langid>>8); uint16_t total=*plen;
    return usb_.ctrlReq(addr,0,bmReqType,bRequest,wValLo,wValHi,wIndLo,wIndHi,total,buf,NULL);
  }
  uint8_t getDeviceDescriptor(uint8_t* buf,uint16_t* len){ return getDescriptor(0x01,0,0,buf,len); }
  uint8_t getConfigDescriptor(uint8_t* buf,uint16_t* len){ uint16_t step=(*len>9)?9:*len; uint8_t r=getDescriptor(0x02,0,0,buf,&step); if(r) return r; uint16_t total=buf[2]|(buf[3]<<8); if(total>*len) total=*len; return getDescriptor(0x02,0,0,buf,&total); }
  uint8_t getStringDescriptor(uint8_t index,uint16_t langid,uint8_t* buf,uint16_t* len){ return getDescriptor(0x03,index,langid,buf,len); }
  uint8_t getMsOsDescriptor(uint8_t* buf,uint16_t* len){ return getDescriptor(0x03,0xEE,0x0000,buf,len); }
private:
  void applyCtrl_(){ if(acm_) acm_->SetControlLineState((dtr_?1:0)|(rts_?2:0)); }
  void pullRx_(){ if(!acm_) return; uint8_t buf[64]; for(;;){ uint16_t len=sizeof(buf); uint8_t r=acm_->RcvData(&len, buf); lastRcvErr_=r; if(r||!len) break; bytesIn_+=len; for(uint16_t i=0;i<len;i++) rx_.push_back(buf[i]); ring_.push(buf,len); if(rx_.size()>16384){ while(rx_.size()>8192) rx_.pop_front(); } } }
  USB usb_; ACM* acm_; bool ready_; std::deque<uint8_t> rx_; uint32_t bytesIn_, bytesOut_; uint8_t lastRcvErr_, lastSndErr_; bool dtr_, rts_; LINE_CODING lc_{115200,0,0,8}; RingLog ring_;
};
