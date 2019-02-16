/*!
 * \file regex_test.cpp
 *
 * \brief 测试正則
 */
#include "../../src/regular_lang/regex.hpp"
#include "../helper.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  using namespace cyy::computation;
  try {
    auto nfa = fuzzing_NFA(Data, Size);
    nfa.to_DFA();
  } catch (const std::invalid_argument &) {
  }
  return 0; // Non-zero return values are reserved for future use.
}
