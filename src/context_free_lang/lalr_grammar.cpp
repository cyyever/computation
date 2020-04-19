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
    auto const unincluded_symbol = alphabet->get_unincluded_symbol();
    item_set.add_kernel_item(*this, item, {unincluded_symbol});

    lookahead_map_type res;
    for (const auto &[grammar_symbol, next_item_set] : GOTO(item_set)) {
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
        res[grammar_symbol][next_kernel_item.production] = {
            propagation, std::move(spontaneous_lookahead_set)};
      }
    }
    return res;
  }

  std::pair<LALR_grammar::collection_type,
            LALR_grammar::goto_transition_set_type>
  LALR_grammar::canonical_collection() const {
    auto [canonical_LR_0_collection, SLR_goto_transitions] =
        SLR_grammar(alphabet->get_name(), start_symbol, productions)
            .canonical_collection();

    std::unordered_map<const LR_0_item *, std::set<CFG::terminal_type>>
        kernel_item_table;
    std::unordered_map<const LR_0_item *, std::vector<const LR_0_item *>>
        propagation_relation;
    std::unordered_map<state_type, const LR_0_item_set *>
        reversed_canonical_LR_0_collection;

    for (auto const &[lr_0_item_set, state] : canonical_LR_0_collection) {
      reversed_canonical_LR_0_collection[state] = &lr_0_item_set;
      for (const auto &kernel_item : lr_0_item_set.get_kernel_items()) {

        if (kernel_item.production.get_head() == new_start_symbol) {
          kernel_item_table[&kernel_item] = {alphabet->get_endmarker()};
        } else {
          kernel_item_table[&kernel_item] = {};
        }
      }
    }

    for (auto const &[lr_0_item_set, state] : canonical_LR_0_collection) {
      for (const auto &kernel_item : lr_0_item_set.get_kernel_items()) {
        for (auto [symbol, lookahead_map] : check_lookahead(kernel_item)) {

          auto const &next_set =
              *reversed_canonical_LR_0_collection[SLR_goto_transitions[{
                  state, symbol}]];
          for (auto const &next_kernel_item : next_set.get_kernel_items()) {
            auto &p = lookahead_map[next_kernel_item.production];
            kernel_item_table[&next_kernel_item].merge(p.second);
            if (p.first) {
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

    for (auto const &[lr_0_item_set, state] : canonical_LR_0_collection) {
      LR_1_item_set item_set;
      for (const auto &kernel_item : lr_0_item_set.get_kernel_items()) {
        item_set.add_kernel_item(*this, kernel_item,
                                 kernel_item_table[&kernel_item]);
      }
      collection.emplace(std::move(item_set), state);
    }
    return {collection, SLR_goto_transitions};
  }

  void LALR_grammar::construct_parsing_table() const {
    try {
      canonical_LR_grammar::construct_parsing_table();
    } catch (const cyy::computation::exception::no_canonical_LR_grammar &e) {
      throw cyy::computation::exception::no_LALR_grammar(e.what());
    }
  }
} // namespace cyy::computation
