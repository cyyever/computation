/*!
 * \file canonical_lr_grammar_fuzzing.cpp
 *
 * \brief
 * \author cyy
 * \date 2019-02-14
 */

#include "../helper.hpp"
#include "context_free_lang/canonical_lr_grammar.hpp"

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
    canonical_LR_grammar grammar("common_tokens", start_symbol, productions);
    static_cast<void>(grammar.parse(
        str, [](auto) {}, [](auto &) {}));
  } catch (const std::invalid_argument &) {
  }
  return 0; // Non-zero return values are reserved for future use.
}
