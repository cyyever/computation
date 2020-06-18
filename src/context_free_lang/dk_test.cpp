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
#include "context_free_lang/lr_item.hpp"
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
    /* for (auto const s : *nfa_alphabet) { */
    /*   transition_function[{start_state, s}].emplace(start_state); */
    /* } */
    auto cur_state = start_state;
    NFA::state_set_type final_states;

    for (auto const &[head, bodies] : productions) {
      for (auto const &body : bodies) {
        /* cyy::computation::LR_0_item production_item({head, body}); */
        /*
        cur_state++;
        if(head==start_state) {
          transition_function[{start_state}].emplace(cur_state);
        }
        for (auto const &grammar_symbol : body) {
          cur_state++;
          if (grammar_symbol.is_terminal()) {
            transition_function[{cur_state - 1, grammar_symbol.get_terminal()}]
                .emplace(cur_state);
          } else {
            transition_function
                [{cur_state - 1,
                  nonterminal_to_symbol[grammar_symbol.get_nonterminal()]}]
                    .emplace(cur_state);
          }
        }
        final_states.insert(cur_state);
        */
      }
    }
  }

} // namespace cyy::computation
