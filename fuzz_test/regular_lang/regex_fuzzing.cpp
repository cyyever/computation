/*!
 * \file regex_test.cpp
 *
 * \brief 测试正則
 */
#include "../helper.hpp"
#include "regular_lang/regex.hpp"

using namespace cyy::computation;
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size);
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  symbol_string expr;
  for (size_t i = 0; i < Size; i++) {
    expr.push_back(static_cast<symbol_type>(to_printable_ASCII(Data[i])));
  }
  try {
    regex reg("printable_ASCII", expr);
    auto nfa = reg.to_NFA();
    auto dfa = reg.to_DFA();
    auto str = reg.get_syntax_tree()->to_string();
  } catch (const std::invalid_argument &) {
  }
  return 0; // Non-zero return values are reserved for future use.
}
