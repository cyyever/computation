/*!
 * \file slr_grammar_fuzzing.cpp
 *
 * \brief
 * \author cyy
 * \date 2019-02-14
 */

#include "../../src/context_free_lang/slr_grammar.hpp"
#include "../helper.hpp"

using namespace cyy::computation;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size);
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  auto part_size = Size / 2;
  auto productions = fuzzing_CFG_productions(Data, part_size);

  if (productions.empty()) {
    return 0;
  }
   auto start_symbol = productions.begin()->first;

  auto str = fuzzing_symbol_string(Data + part_size, Size - part_size);
  try {
    SLR_grammar grammar("common_tokens", start_symbol, productions);
    static_cast<void>(grammar.parse(
        str, [](auto) {}, [](auto &) {}));
  } catch (const std::invalid_argument &) {
  }
  return 0; // Non-zero return values are reserved for future use.
}
