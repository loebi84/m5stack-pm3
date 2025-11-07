
#pragma once
#include <deque>
#include <Arduino.h>
class RingLog{
  std::deque<uint8_t> buf_; size_t cap_;
public:
  explicit RingLog(size_t cap=8192):cap_(cap){}
  void push(const uint8_t* d, size_t n){ for(size_t i=0;i<n;i++){ if(buf_.size()>=cap_) buf_.pop_front(); buf_.push_back(d[i]); } }
  String dumpHex(size_t max=512, bool tail=true) const {
    String out; 
    if(buf_.empty()) return out;
    size_t n = min(max, buf_.size()); 
    size_t start = tail? buf_.size()-n : 0;
    out.reserve(n*3); // Pre-allocate for efficiency
    static const char hex[] = "0123456789ABCDEF";
    for(size_t i=0;i<n;i++){ 
      if(i) out+=' '; 
      uint8_t b = buf_[start+i];
      out += hex[b >> 4];
      out += hex[b & 0x0F];
    } 
    return out;
  }
  void clear(){ buf_.clear(); }
  size_t size() const { return buf_.size(); }
};
