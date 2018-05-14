/*!
 * \file regex_test.cpp
 *
 * \brief 测试正則
 */
#include <iostream>

#include "../src/ascii.hpp"
#include "../src/regex.hpp"

using namespace cyy::lang;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  symbol_string expr;

  for (size_t i = 0; i < Size; i++) {
    expr.push_back(Data[i]%95+32);
  }
  //try {
    regex reg("printable-ASCII", expr);
 // } catch (const std::runtime_error &) {
 // }
  return 0; // Non-zero return values are reserved for future use.
}
