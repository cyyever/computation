/*!
 * \file slr_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */
#include <iostream>
#include <unordered_map>

#include "../exception.hpp"
#include "slr_grammar.hpp"

namespace cyy::computation {

  void SLR_grammar::construct_parsing_table() const {
    const_cast<SLR_grammar *>(this)->normalize_start_symbol();

    auto [dk, _, symbol_to_nonterminal, state_to_item_set] = get_DK();
    for (auto const &[situation, next_state] : dk.get_transition_function()) {
      assert(state_to_item_set.contans(next_state));
      if (state_to_item_set[next_state].empty()) {
        continue;
      }
      auto it = symbol_to_nonterminal.find(situation.input_symbol);
      if (it != symbol_to_nonterminal.end()) {
        goto_table[{situation.state, it->second}] = next_state;
      } else {
        std::cout << "shift of state " << situation.state << " and input_symbol"
                  << (int)situation.input_symbol << std::endl;
        action_table[{situation.state, situation.input_symbol}] = next_state;
      }
    }

    auto follow_sets = follow();
    for (auto const &[state, set] : state_to_item_set) {
      for (const auto &kernel_item : set.get_kernel_items()) {
        if (!kernel_item.completed()) {
          continue;
        }

        std::cout << "completed state is " << state << std::endl;
        if (kernel_item.get_head() == start_symbol) {
          action_table[{state, ALPHABET::endmarker}] = true;
          continue;
        }

        for (auto const &follow_terminal :
             follow_sets[kernel_item.get_head()]) {

          // conflict
          if (action_table.contains({state, follow_terminal})) {
            std::cerr << "conflict with follow_terminal "
                      << alphabet->to_string(follow_terminal) << std::endl;
            throw cyy::computation::exception::no_SLR_grammar("");
          }
          action_table[{state, follow_terminal}] = kernel_item.get_production();
        }
      }
    }
    const_cast<SLR_grammar *>(this)->remove_head(start_symbol,
                                                 old_start_symbol);
  }
} // namespace cyy::computation
