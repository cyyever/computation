/*!
 * \file slr_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "slr_grammar.hpp"

namespace cyy::computation {

  std::pair<SLR_grammar::collection_type, SLR_grammar::goto_transition_map_type>
  SLR_grammar::get_collection() const {
    goto_transition_map_type _goto_table;
    auto [dk, _, symbol_to_nonterminal, state_to_item_set] = get_DK();
    for (auto const &[situation, next_state] : dk.get_transition_function()) {
      assert(state_to_item_set.contans(next_state));
      if (state_to_item_set[next_state].empty()) {
        continue;
      }
      auto it = symbol_to_nonterminal.find(situation.input_symbol);
      if (it != symbol_to_nonterminal.end()) {
        _goto_table[{situation.state, it->second}] = next_state;
      } else {
        _goto_table[{situation.state, situation.input_symbol}] = next_state;
      }
    }
    collection_type collection;
    auto follow_sets = follow();
    for (auto const &[state, lr_0_item_set] : state_to_item_set) {
      LR_1_item_set set;
      for (auto const &item : lr_0_item_set.get_kernel_items()) {
        if (item.completed()) {
          set.add_kernel_item(*this, item, follow_sets[item.get_head()]);
        }
      }
      collection[state] = set;
    }
    return {collection, _goto_table};
  }
} // namespace cyy::computation
