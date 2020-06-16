/*!
 * \file grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <utility>

#include "cfg.hpp"
#include "lang/range_alphabet.hpp"
#include "lang/union_alphabet.hpp"
#include "regular_lang/nfa.hpp"

namespace cyy::computation {

  void CFG::dk_test() const {
    auto max_symbol = alphabet->get_max_symbol();
    auto nonterminals = get_nonterminals();

    auto alphabet_of_nonterminals = std::make_shared<range_alphabet>(
        max_symbol + 1, max_symbol + nonterminals.size(),
        "alphabet_of_nonterminals");
    std::unordered_map<nonterminal_type, symbol_type> nonterminal_to_symbol;
    max_symbol++;
    for (auto const &nonterminal : nonterminals) {
      nonterminal_to_symbol.emplace(nonterminal, max_symbol);
      max_symbol++;
    }

    auto nfa_alphabet =
        std::make_shared<union_alphabet>(alphabet, alphabet_of_nonterminals);

    using state_type = NFA::state_type;

    state_type start_state = 0;
    NFA::transition_function_type transition_function;
    for (auto const s : *nfa_alphabet) {
      transition_function[{start_state, s}].emplace(start_state);
    }
    auto next_state = start_state + 1;
    for (auto const &[head, bodies] : productions) {
    }
  }

} // namespace cyy::computation
