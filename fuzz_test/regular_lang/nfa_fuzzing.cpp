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
    auto part_size = Size / 2;
    auto nfa = fuzzing_NFA(Data, part_size);
    auto str = fuzzing_symbol_string(Data + part_size, Size - part_size);
    nfa.recognize(str);
    auto dfa = nfa.to_DFA();
    auto gnfa = GNFA(dfa);
  } catch (const std::invalid_argument &) {
  }
  return 0; // Non-zero return values are reserved for future use.
}
