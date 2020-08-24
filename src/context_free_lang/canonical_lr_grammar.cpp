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
  canonical_LR_grammar::canonical_LR_grammar(ALPHABET_ptr alphabet_,
                                             nonterminal_type start_symbol_,
                                             production_set_type productions_)

      : LR_1_grammar(alphabet_, start_symbol_, std::move(productions_)),
        dk_1_dfa_ptr(std::make_shared<DK_1_DFA>(*this)) {
    if (!DK_test()) {
      throw exception::no_LR_1_grammar("DK test failed");
    }
  }

  bool canonical_LR_grammar::DK_test() const {
    for (auto &[_, item_set] : dk_1_dfa_ptr->get_LR_1_item_set_collection()) {
      if (!item_set.has_completed_items()) {
        continue;
      }
      std::vector<LR_1_item> uncompleted_items;
      std::vector<LR_1_item> completed_items;
      for (auto const &item : item_set.get_kernel_items()) {
        if (item.completed()) {
          completed_items.emplace_back(item);
          continue;
        }
        uncompleted_items.emplace_back(item);
      }
      for (auto const &item : item_set.expand_nonkernel_items(*this)) {
        uncompleted_items.emplace_back(item);
      }
      for (auto it = completed_items.begin(); it != completed_items.end();
           it++) {
        for (auto it2 = it + 1; it2 != completed_items.end(); it2++) {
          if (std::ranges::includes(it->get_lookahead_symbols(),
                                    it2->get_lookahead_symbols())) {
            return false;
          }
        }
      }
      for (auto &uncompleted_item : uncompleted_items) {
        if (!uncompleted_item.get_grammar_symbal().is_terminal()) {
          continue;
        }
        auto terminal = uncompleted_item.get_grammar_symbal().get_terminal();

        for (auto &completed_item : completed_items) {
          if (completed_item.get_lookahead_symbols().contains(terminal)) {
            return false;
          }
        }
      }
    }
    return true;
  }

  std::pair<canonical_LR_grammar::collection_type,
            canonical_LR_grammar::goto_table_type>
  canonical_LR_grammar::get_collection() const {

    LR_1_item_set init_set;
    init_set.add_kernel_item(
        *this,
        LR_0_item{CFG_production{get_start_symbol(),
                                 get_bodies(get_start_symbol())[0]},
                  0},
        {ALPHABET::endmarker});
    std::unordered_map<LR_1_item_set, state_type> unchecked_sets,
        reversed_collection;
    unchecked_sets.emplace(std::move(init_set), 0);

    state_type next_state = 1;

    goto_table_type goto_transitions;
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
