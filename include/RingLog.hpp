
#pragma once
#include <deque>
#include <Arduino.h>
class RingLog{
  std::deque<uint8_t> buf_; size_t cap_;
public:
  explicit RingLog(size_t cap=8192):cap_(cap){}
  void push(const uint8_t* d, size_t n){ for(size_t i=0;i<n;i++){ if(buf_.size()>=cap_) buf_.pop_front(); buf_.push_back(d[i]); } }
  String dumpHex(size_t max=512, bool tail=true) const {
    String out; if(buf_.empty()) return out;
    size_t n = min(max, buf_.size()); size_t start = tail? buf_.size()-n : 0;
    for(size_t i=0;i<n;i++){ if(i) out+=' '; char b[4]; sprintf(b,"%02X", buf_[start+i]); out+=b; } return out;
  }
  void clear(){ buf_.clear(); }
  size_t size() const { return buf_.size(); }
};
