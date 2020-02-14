/*!
 * \file lalr_grammar_fuzzing.cpp
 *
 * \brief
 * \author cyy
 * \date 2019-02-14
 */

#include "../../src/context_free_lang/lalr_grammar.hpp"
#include "../../src/lang/common_tokens.hpp"
#include "../helper.hpp"

using namespace cyy::computation;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size);
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  auto part_size = Size / 2;
  auto productions = fuzzing_CFG_productions(Data, part_size);
  CFG::nonterminal_type start_symbol;
  if (!productions.empty()) {
    start_symbol = productions.begin()->first;
  }

  try {
    CFG cfg("common_tokens", start_symbol, productions);

    auto str = fuzzing_symbol_string(Data + part_size, Size - part_size);
    static_cast<void>(cfg.recursive_descent_parse(str));
  } catch (const std::invalid_argument &) {
  }
  return 0; // Non-zero return values are reserved for future use.
}
