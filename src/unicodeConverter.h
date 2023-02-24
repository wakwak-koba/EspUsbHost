/*
  https://twitter.com/wakwak_koba
*/
#ifndef _WAKWAK_KOBA_UNICODE_CONVERTER_HPP_
#define _WAKWAK_KOBA_UNICODE_CONVERTER_HPP_

#include <string>

namespace utf {
  std::u16string toString16(const std::u32string input, const size_t length = SIZE_MAX);  // UTF32 to UTF16
  std::string toString(const std::u16string input, const size_t length = SIZE_MAX);       // UTF16 to UTF8
  std::u32string toString32(const std::string input, const size_t length = SIZE_MAX);     // UTF8 to UTF32
  std::u16string toString16(const std::string input, const size_t length = SIZE_MAX);     // UTF8 to UTF16
  std::u32string toString32(const std::u16string input, const size_t length = SIZE_MAX);  // UTF16 to UTF32
  std::string toString(const std::u32string input, const size_t length = SIZE_MAX);       // UTF32 to UTF8
}

#endif