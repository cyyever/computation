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

      : LR_1_grammar(alphabet_, start_symbol_, std::move(productions_)) {}

  bool canonical_LR_grammar::DK_1_test(
      const DK_1_DFA::LR_1_item_set_collection_type &collection) const {
    for (auto &[_, item_set] : collection) {
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
    DK_1_DFA dk_1_dfa(*this);
    auto const &collection = dk_1_dfa.get_LR_1_item_set_collection();
    if (!DK_1_test(collection)) {
      throw exception::no_LR_1_grammar("DK 1 test failed");
    }
    auto goto_table = dk_1_dfa.get_goto_table();
    return {collection, goto_table};
  }

} // namespace cyy::computation
