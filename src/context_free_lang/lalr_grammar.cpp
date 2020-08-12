/*!
 * \file lalr_grammar.cpp
 *
 * \author cyy
 * \date 2018-09-24
 */

#include "lalr_grammar.hpp"
#include "../exception.hpp"
#include "slr_grammar.hpp"

namespace cyy::computation {
  LALR_grammar::lookahead_map_type
  LALR_grammar::check_lookahead(const LR_0_item &item) const {
    LR_1_item_set item_set;
    auto const unincluded_symbol = ALPHABET::endmarker;
    assert(!(contains(unincluded_symbol)));
    item_set.add_kernel_item(*this, item, {unincluded_symbol});

    lookahead_map_type res;
    for (const auto &[grammar_symbol, next_item_set] : item_set.go(*this)) {
      for (auto const &[next_kernel_item, lookahead_set] :
           next_item_set.get_kernel_items()) {
        bool propagation = false;
        std::set<CFG::terminal_type> spontaneous_lookahead_set;
        if (lookahead_set.contains(unincluded_symbol)) {
          propagation = true;
        }
        spontaneous_lookahead_set.insert(lookahead_set.begin(),
                                         lookahead_set.end());
        spontaneous_lookahead_set.erase(unincluded_symbol);
        res[grammar_symbol][next_kernel_item.get_production()] = {
            propagation, std::move(spontaneous_lookahead_set)};
      }
    }
    return res;
  }

  std::pair<LALR_grammar::collection_type, LALR_grammar::goto_table_type>
  LALR_grammar::get_collection() const {
    goto_table_type _goto_table;
    auto [dk, _, symbol_to_nonterminal, state_to_item_set] = get_DK();
    for (auto const &[situation, next_state] : dk.get_transition_function()) {
      assert(state_to_item_set.contains(next_state));
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
    std::unordered_map<const LR_0_item *, std::set<CFG::terminal_type>>
        kernel_item_table;

    for (auto const &[state, lr_0_item_set] : state_to_item_set) {
      for (const auto &kernel_item : lr_0_item_set.get_kernel_items()) {
        if (kernel_item.get_production().get_head() == start_symbol) {
          kernel_item_table[&kernel_item] = {ALPHABET::endmarker};
        } else {
          kernel_item_table[&kernel_item] = {};
        }
      }
    }

    std::unordered_map<const LR_0_item *, std::vector<const LR_0_item *>>
        propagation_relation;
    for (auto const &[state, lr_0_item_set] : state_to_item_set) {
      for (const auto &kernel_item : lr_0_item_set.get_kernel_items()) {
        for (auto [symbol, lookahead_map] : check_lookahead(kernel_item)) {
          assert(_goto_table.contains({state, symbol}));
          assert(state_to_item_set.contains(_goto_table[{state, symbol}]));
          auto const &next_set =
              state_to_item_set[_goto_table[{state, symbol}]];
          for (auto const &next_kernel_item : next_set.get_kernel_items()) {
            auto it = lookahead_map.find(next_kernel_item.get_production());
            assert(it != lookahead_map.end());
            assert(kernel_item_table.contains(&next_kernel_item));
            kernel_item_table[&next_kernel_item].merge(
                std::move(it->second.second));
            if (it->second.first) {
              propagation_relation[&kernel_item].push_back(&next_kernel_item);
            }
          }
        }
      }
    }

    while (true) {
      bool new_lookhead = false;
      for (auto const &[kernel_item_ptr, lookahead_set] : kernel_item_table) {
        for (auto const &next_kernel_item_ptr :
             propagation_relation[kernel_item_ptr]) {
          for (auto const &lookahead_symbol : lookahead_set) {
            assert(kernel_item_table.contains(next_kernel_item_ptr));
            if (kernel_item_table[next_kernel_item_ptr]
                    .emplace(lookahead_symbol)
                    .second) {
              new_lookhead = true;
            }
          }
        }
      }
      if (!new_lookhead) {
        break;
      }
    }

    collection_type collection;

    for (auto const &[state, lr_0_item_set] : state_to_item_set) {
      LR_1_item_set item_set;
      for (const auto &kernel_item : lr_0_item_set.get_kernel_items()) {
        assert(kernel_item_table.contains(&kernel_item));
        item_set.add_kernel_item(*this, kernel_item,
                                 kernel_item_table[&kernel_item]);
      }
      collection.emplace(state, std::move(item_set));
    }
    return {collection, _goto_table};
  }

} // namespace cyy::computation
