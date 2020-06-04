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

  std::unordered_map<grammar_symbol_type, LR_0_item_set>
  SLR_grammar::GOTO(const LR_0_item_set &set) const {
    std::unordered_map<grammar_symbol_type, LR_0_item_set> res;

    for (auto const &kernel_item : set.get_kernel_items()) {
      if (kernel_item.dot_pos < kernel_item.production.get_body().size()) {
        auto const &symbol =
            kernel_item.production.get_body()[kernel_item.dot_pos];
        auto new_kernel_item = kernel_item;
        new_kernel_item.dot_pos++;
        res[symbol].add_kernel_item(*this, std::move(new_kernel_item));
      }
    }

    for (auto const &nonkernel_item : set.get_nonkernel_items()) {
      auto it = productions.find(nonkernel_item);

      for (auto const &body : it->second) {
        if (body.empty()) {
          continue;
        }
        LR_0_item new_kernel_item{{nonkernel_item, body}, 1};
        res[body[0]].add_kernel_item(*this, std::move(new_kernel_item));
      }
    }

    return res;
  }

  std::pair<SLR_grammar::collection_type, SLR_grammar::goto_transition_set_type>
  SLR_grammar::canonical_collection() const {
    collection_type unchecked_sets;
    collection_type collection;
    goto_transition_set_type goto_transitions;

    LR_0_item_set init_set;
    init_set.add_kernel_item(
        *this, LR_0_item{CFG_production{new_start_symbol, {start_symbol}}, 0});

    unchecked_sets.emplace(std::move(init_set), 0);

    state_type next_state = 1;

    while (!unchecked_sets.empty()) {

      auto node = unchecked_sets.extract(unchecked_sets.begin());

      auto cur_state = node.mapped();

      auto next_sets = GOTO(node.key());
      collection.emplace(std::move(node.key()), cur_state);

      for (auto &[symbol, next_set] : next_sets) {
        auto it = collection.find(next_set);
        if (it == collection.end()) {
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
    return {collection, goto_transitions};
  }

  void SLR_grammar::construct_parsing_table() const {
    auto [collection, goto_transitions] = canonical_collection();
    auto follow_sets = follow();
    auto endmarker = alphabet->get_endmarker();

    for (auto const &[p, next_state] : goto_transitions) {
      auto ptr = p.second.get_nonterminal_ptr();
      if (ptr) {
        goto_table[{p.first, *ptr}] = next_state;
      } else {
        action_table[{p.first, p.second.get_terminal()}] = next_state;
      }
    }

    for (auto const &[set, state] : collection) {
      for (const auto &kernel_item : set.get_kernel_items()) {
        if (kernel_item.dot_pos != kernel_item.production.get_body().size()) {
          continue;
        }

        if (kernel_item.production.get_head() == new_start_symbol) {
          action_table[{state, endmarker}] = true;
          continue;
        }

        for (auto const &follow_terminal :
             follow_sets[kernel_item.production.get_head()]) {

          // conflict
          if (action_table.contains({state, follow_terminal})) {
            std::cerr << "conflict with follow_terminal ";
            alphabet->to_string(follow_terminal);
            std::cerr << std::endl;
            throw cyy::computation::exception::no_SLR_grammar("");
          }
          action_table[{state, follow_terminal}] = kernel_item.production;
        }
      }
    }
  }
} // namespace cyy::computation
