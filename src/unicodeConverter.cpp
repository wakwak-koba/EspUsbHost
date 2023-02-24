/*
  https://twitter.com/wakwak_koba
*/
#include "unicodeConverter.h"

namespace utf {
  //
  // UTF32 to UTF8
  //
  std::string toString(const std::u32string input, const size_t length) {
    std::string result;

    for(int i = 0; i < std::min(length, input.length()); i++) {
      char32_t u32ch = input.c_str()[i];

      if (u32ch < 0 || u32ch > 0x10ffff)
        break;

      if (u32ch < 0x80) {
        result += (char)u32ch;
      } else if (u32ch < 0x800) {
        result += (char)(0xc0 |  (u32ch >>  6)        );
        result += (char)(0x80 |  (u32ch        & 0x3f));
      } else if (u32ch < 0x10000) {
        result += (char)(0xe0 |  (u32ch >> 12)        );
        result += (char)(0x80 | ((u32ch >>  6) & 0x3f));
        result += (char)(0x80 |  (u32ch        & 0x3f));
      } else {
        result += (char)(0xf0 |  (u32ch >> 18)        );
        result += (char)(0x80 | ((u32ch >> 12) & 0x3f));
        result += (char)(0x80 | ((u32ch >>  6) & 0x3f));
        result += (char)(0x80 |  (u32ch        & 0x3f));
      }
    }
    return result;
  }

  //
  // UTF16 to UTF32
  //
  std::u32string toString32(const std::u16string input, const size_t length) {
    std::u32string result;

    char32_t IsSurrogateH = 0;
    char32_t IsSurrogateL = 0;
    for(int i = 0; i < std::min(length, input.length()); i++) {
      char32_t u16ch = input.c_str()[i];
      if(IsSurrogateH) {
        if(0xdc00 <= u16ch && u16ch < 0xe000)
          result += (char32_t)(0x10000 + (IsSurrogateH - 0xd800) * 0x400 + (u16ch - 0xdc00));
        else if(!u16ch)
          result += IsSurrogateH;
        IsSurrogateH = 0;
      } else if (IsSurrogateL) {
        if(!u16ch)
          result += IsSurrogateL;
        IsSurrogateL = 0;
      } else if(u16ch < 0xd800 || u16ch >= 0xe000)
        result += u16ch;
      else if(u16ch < 0xdc00)
        IsSurrogateH = u16ch;
      else
        IsSurrogateL = u16ch;
    }
    return result;
  }

  std::string toString(const std::u16string input, const size_t length) {
    return toString(toString32(input, length));
  }

  std::u32string toString32(const std::string input, const size_t length) {
    std::u32string result;
    uint32_t buf[4];
    int pos = 0;

    for(int i = 0; i < std::min(length, input.length()) && pos < sizeof(pos); i++) {
      buf[pos ++] = input.c_str()[i];

      if (buf[0] < 0x80) {
        result += (char32_t)buf[0];
        pos = 0;
      }
      else if(buf[0] < 0xc2 || buf[0] >= 0xf8)
        break;
      else if (buf[0] < 0xe0 && pos > 1) {
        if(buf[1] < 0x80 || buf[1] >= 0xc0 || !(buf[0] & 0x1e))
          break;
        result += (char32_t)( ((buf[0] & 0x1f) << 6 ) | (buf[1] & 0x3f) );
        pos = 0;
      }
      else if (buf[0] < 0xf0 && pos > 2) {
        if(buf[1] < 0x80 || buf[1] >= 0xc0 || buf[2] < 0x80 || buf[2] >= 0xc0 || (!(buf[0] & 0x0f) && !(buf[1] & 0x20)))
          break;
        result += (char32_t)( ((buf[0] & 0x0f) << 12) | ((buf[1] & 0x3f) << 6) | (buf[2] & 0x3f) );
        pos = 0;
      }
      else if (buf[0] < 0xf8 && pos > 3) {
        if(buf[1] < 0x80 || buf[1] >= 0xc0 || buf[2] < 0x80 || buf[2] >= 0xc0 || buf[3] < 0x80 || buf[3] >= 0xc0 || (!(buf[0] & 0x07) && !(buf[1] & 0x30)))
        result += (char32_t)( ((buf[0] & 0x07) << 18) | ((buf[1] & 0x3f) << 12) | ((buf[2] & 0x3f) << 6) | (buf[3] & 0x3f) );
        pos = 0;
      }
    }
    return result; 
  }

  std::u16string toString16(const std::u32string input, const size_t length) {
    std::u16string result;

    for(int i = 0; i < std::min(input.length(), length); i++) {
      char32_t u32Ch = input.c_str()[i];
      if (u32Ch < 0 || u32Ch > 0x10ffff)
          break;

      if (u32Ch < 0x10000) {
        result += (char16_t)u32Ch;
      } else {
        result += char16_t((u32Ch - 0x10000) / 0x400 + 0xd800);
        result += char16_t((u32Ch - 0x10000) % 0x400 + 0xdc00);
      }    
    }
    return result;
  }

  std::u16string toString16(const std::string input, const size_t length) {
    return toString16(toString32(input, length));
  }
}
