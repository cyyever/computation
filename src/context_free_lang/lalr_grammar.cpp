/*!
 * \file lalr_grammar.cpp
 *
 * \author cyy
 * \date 2018-09-24
 */

#include "lalr_grammar.hpp"

#include "exception.hpp"

namespace cyy::computation {
  std::pair<LALR_grammar::collection_type, LALR_grammar::goto_table_type>
  LALR_grammar::get_collection() const {
    auto collection_pair = canonical_LR_grammar::get_collection();
    auto &[collection, goto_table] = collection_pair;

    std::unordered_map<LR_0_item_set, DFA::state_set_type> state_sets;
    for (auto const &[state, lr_1_item_set] : collection) {
      state_sets[lr_1_item_set.get_lr_0_item_set()].emplace(state);
    }
    std::unordered_map<DFA::state_type, DFA::state_type> state_map;
    collection_type new_collection;
    for (auto const &[_, state_set] : state_sets) {
      assert(!state_set.empty());
      auto it = state_set.begin();
      auto min_state = *it;
      auto &item_set = collection[min_state];
      ++it;
      state_map[min_state] = min_state;
      for (; it != state_set.end(); ++it) {
        item_set.merge_lookahead_symbols(collection[*it]);
        state_map[*it] = min_state;
      }
      new_collection.try_emplace(min_state, std::move(item_set));
    }

    goto_table_type new_goto_table;

    for (auto [from_config, to_state] : goto_table) {
      new_goto_table.try_emplace(
          {state_map[from_config.first], from_config.second},
          state_map[to_state]);
    }
    return {new_collection, new_goto_table};
  }

} // namespace cyy::computation
