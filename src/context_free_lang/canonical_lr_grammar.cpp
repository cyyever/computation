/*!
 * \file canonical_lr_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include <iostream>
#include <unordered_map>

#include "../exception.hpp"
#include "canonical_lr_grammar.hpp"

namespace cyy::computation {

  std::pair<canonical_LR_grammar::collection_type,
            canonical_LR_grammar::goto_transition_map_type>
  canonical_LR_grammar::get_collection() const {

    LR_1_item_set init_set;
    init_set.add_kernel_item(
        *this, LR_0_item{CFG_production{start_symbol, {old_start_symbol}}, 0},
        {ALPHABET::endmarker});
    std::unordered_map<LR_1_item_set, state_type> unchecked_sets,
        reversed_collection;
    unchecked_sets.emplace(std::move(init_set), 0);

    state_type next_state = 1;

    goto_transition_map_type goto_transitions;
    while (!unchecked_sets.empty()) {

      auto node = unchecked_sets.extract(unchecked_sets.begin());

      auto cur_state = node.mapped();

      auto next_sets = node.key().go(*this);
      reversed_collection.emplace(std::move(node.key()), cur_state);

      for (auto &[symbol, next_set] : next_sets) {
        auto it = reversed_collection.find(next_set);
        if (it == reversed_collection.end()) {
          it = unchecked_sets.find(next_set);
          if (it == unchecked_sets.end()) {
            unchecked_sets.emplace(std::move(next_set), next_state);
            goto_transitions[{cur_state, symbol}] = next_state;
            next_state++;
            continue;
          }
        }
        goto_transitions[{cur_state, symbol}] = it->second;
      }
    }
    collection_type collection;
    for (auto &[k, v] : reversed_collection) {
      collection.emplace(std::move(v), std::move(k));
    }
    return {collection, goto_transitions};
  }

} // namespace cyy::computation
