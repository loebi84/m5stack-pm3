
#pragma once
#include <Arduino.h>
#include <vector>
inline bool hexToBytes(const String& hex, std::vector<uint8_t>& out){
  out.clear(); int n=hex.length(), i=0;
  auto hv=[](char c)->int{ if(c>='0'&&c<='9')return c-'0'; if(c>='a'&&c<='f')return c-'a'+10; if(c>='A'&&c<='F')return c-'A'+10; return -1; };
  while(i<n){ while(i<n && isspace((int)hex[i])) i++; if(i<n && hex[i]=='0' && i+1<n && (hex[i+1]=='x'||hex[i+1]=='X')){ i+=2; continue; }
    if(i>=n)break; if(i+1>=n)return false; int hi=hv(hex[i++]); int lo=hv(hex[i++]); if(hi<0||lo<0)return false; out.push_back((uint8_t)((hi<<4)|lo)); }
  return true;
}
inline String bytesToHexLine(const uint8_t* d, size_t n){
  String s; 
  if(n==0) return s;
  s.reserve(n*3); // Pre-allocate: 2 chars per byte + spaces
  static const char hex[] = "0123456789ABCDEF";
  for(size_t i=0;i<n;i++){ 
    if(i) s+=' '; 
    s += hex[d[i] >> 4];
    s += hex[d[i] & 0x0F];
  }
  return s;
}
