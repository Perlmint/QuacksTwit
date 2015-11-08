#pragma once
#include "color.h"
#include "rapidjson/document.h"

inline unsigned short hex2dec(char ch)
{
  ch = ch | 0x20;
  if (ch >= '0' && ch <= '9')
  {
    return ch - '0';
  }
  else if (ch >= 'a' && ch <= 'f')
  {
    return ch - 'a' + 10;
  }
  return 0;
}

inline void ParseColor(color &ret, const rapidjson::Value &val)
{
  const char *colorStr = val.GetString();
  ret.red = hex2dec(colorStr[0]) * 16 + hex2dec(colorStr[1]);
  ret.green = hex2dec(colorStr[2]) * 16 + hex2dec(colorStr[3]);
  ret.blue = hex2dec(colorStr[4]) * 16 + hex2dec(colorStr[5]);
}
