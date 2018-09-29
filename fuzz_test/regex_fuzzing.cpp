/*!
 * \file regex_test.cpp
 *
 * \brief 测试正則
 */
#include <iostream>

#include "../src/lang/ascii.hpp"
#include "../src/regular_lang/regex.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  cyy::computation::symbol_string expr;

  for (size_t i = 0; i < Size; i++) {
    expr.push_back(Data[i] % 95 + 32);
  }
  try {
    cyy::computation::regex reg("printable-ASCII", expr);
  } catch (const std::invalid_argument &) {
  }
  return 0; // Non-zero return values are reserved for future use.
}
