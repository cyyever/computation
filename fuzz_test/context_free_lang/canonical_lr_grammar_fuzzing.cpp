/*!
 * \file canonical_lr_grammar_fuzzing.cpp
 *
 * \brief
 * \author cyy
 * \date 2019-02-14
 */

#include "../../src/context_free_lang/canonical_lr_grammar.hpp"
#include "../../src/lang/common_tokens.hpp"
#include "../helper.hpp"

using namespace cyy::computation;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  auto productions = fuzzing_CFG_productions(Data, Size);

  CFG::nonterminal_type start_symbol;
  if (!productions.empty()) {
    start_symbol = productions.begin()->first;
  }

  try {
    canonical_LR_grammar grammar("common_tokens", start_symbol, productions);
  } catch (const std::invalid_argument &) {
  }
  return 0; // Non-zero return values are reserved for future use.
}
