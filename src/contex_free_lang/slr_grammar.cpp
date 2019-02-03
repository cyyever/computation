/*!
 * \file slr_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "slr_grammar.hpp"
#include "../exception.hpp"

namespace cyy::computation {

std::map<grammar_symbol_type, LR_0_item_set>
SLR_grammar::GOTO(const LR_0_item_set &set) const {
  std::map<grammar_symbol_type, LR_0_item_set> res;

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
      LR_0_item new_kernel_item{{nonkernel_item, body}, 1};
      /*
      new_kernel_item.production.get_head() = nonkernel_item;
      new_kernel_item.production.get_body() = body;
      new_kernel_item.dot_pos = 1;
      */
      res[body[0]].add_kernel_item(*this, std::move(new_kernel_item));
    }
  }

  return res;
}

std::pair<std::unordered_map<LR_0_item_set, uint64_t>,
          std::map<std::pair<uint64_t, grammar_symbol_type>, uint64_t>>
SLR_grammar::canonical_collection() const {

  std::unordered_map<LR_0_item_set, uint64_t> unchecked_sets;
  std::unordered_map<LR_0_item_set, uint64_t> collection;
  std::map<std::pair<uint64_t, grammar_symbol_type>, uint64_t> goto_transitions;

  LR_0_item_set init_set;
  init_set.add_kernel_item(
      *this, LR_0_item{CFG_production{new_start_symbol, {start_symbol}}, 0});

  unchecked_sets.emplace(std::move(init_set), 0);

  uint64_t next_state = 1;

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
    if (auto ptr = p.second.get_nonterminal_ptr(); ptr) {
      goto_table[{p.first, *ptr}] = next_state;
    } else if (auto ptr = p.second.get_terminal_ptr(); ptr) {
      action_table[{p.first, *ptr}] = next_state;
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
        if (action_table.count({state, follow_terminal}) != 0) {
          std::cerr << "conflict with follow_terminal ";
          alphabet->print(std::cerr, follow_terminal);
          std::cerr << std::endl;
          throw cyy::computation::exception::no_SLR_grammar("");
        }
        action_table[{state, follow_terminal}] = kernel_item.production;
      }
    }
  }
}
} // namespace cyy::computation
