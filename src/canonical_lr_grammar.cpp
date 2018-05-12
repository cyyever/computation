/*!
 * \file canonical_lr_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "canonical_lr_grammar.hpp"
#include "exception.hpp"

namespace cyy::computation {

std::map<CFG::grammar_symbol_type, LR_1_item_set>
  canonical_LR_grammar::GOTO(const LR_1_item_set &set) const {

std::map<CFG::grammar_symbol_type, LR_1_item_set> res;

  for (auto const &[kernel_item, lookahead_set] : set.get_kernel_items()) {
    if (kernel_item.dot_pos < kernel_item.production.second.size()) {
      auto const &symbol = kernel_item.production.second[kernel_item.dot_pos];
      auto new_kernel_item = kernel_item;
      new_kernel_item.dot_pos++;
      res[symbol].add_kernel_item(*this, new_kernel_item, lookahead_set);
    }
  }

  for (auto const &[nonterminal, lookahead_set] : set.get_nonkernel_items()) {
    auto it = productions.find(nonterminal);

    for (auto const &body : it->second) {

        LR_0_item new_kernel_item;
        new_kernel_item.production.first = nonterminal;
        new_kernel_item.production.second = body;
        new_kernel_item.dot_pos = 1;

        res[body[0]].add_kernel_item(*this, new_kernel_item, lookahead_set);
      }
  }
  return res;
}

std::pair<
    std::vector<LR_1_item_set>,
    std::map<std::pair<uint64_t, canonical_LR_grammar::grammar_symbol_type>,
             uint64_t>>
canonical_LR_grammar::canonical_collection() {
  std::vector<LR_1_item_set> collection;
  std::vector<bool> check_flag{true};
  std::map<std::pair<uint64_t, grammar_symbol_type>, uint64_t> goto_transitions;
  auto endmarker = alphabet->get_endmarker();

  LR_1_item_set init_set;
  init_set.add_kernel_item(
      *this, LR_0_item{production_type{new_start_symbol, {start_symbol}}, 0},
      {endmarker});
  collection.emplace_back(init_set);


  uint64_t next_state = 1;
  for (size_t i = 0; i < check_flag.size(); i++) {
    if (!check_flag[i]) {
      continue;
    }

    auto next_sets= GOTO(collection[i]);

    for(auto &[symbol,next_set]:next_sets) {

      auto it = std::find(collection.begin(), collection.end(), next_set);
      if (it == collection.end()) {
        collection.emplace_back(std::move(next_set));
        check_flag.emplace_back(true);
        goto_transitions[{i, symbol}] = next_state;
        next_state++;
      } else {
        goto_transitions[{i, symbol}] = it - collection.begin();
      }
    }

    check_flag[i] = false;
  }

  return {collection, goto_transitions};
}

void canonical_LR_grammar::construct_parsing_table() {
  auto [collection, goto_transitions] = canonical_collection();
  auto endmarker = alphabet->get_endmarker();

  for (auto const &[p, next_state] : goto_transitions) {
    if (auto ptr = std::get_if<nonterminal_type>(&p.second); ptr) {
      goto_table[{p.first, *ptr}] = next_state;
    } else if (auto ptr = std::get_if<terminal_type>(&p.second); ptr) {
      action_table[{p.first, *ptr}] = next_state;
    }
  }

  for (uint64_t i = 0; i < collection.size(); i++) {
    auto &set = collection[i];

    for (const auto &[kernel_item, lookahead_set] : set.get_kernel_items()) {
      if (kernel_item.dot_pos != kernel_item.production.second.size()) {
        continue;
      }

      if (kernel_item.production.first == new_start_symbol) {
        action_table[{i, endmarker}] = true;
        continue;
      }

      for (const auto &lookahead : lookahead_set) {
        // conflict
        if (action_table.count({i, lookahead}) != 0) {
          std::cout << "config with follow_terminal" << lookahead << std::endl;
          throw cyy::computation::exception::no_canonical_LR_grammar("");
        }
        action_table[{i, lookahead}] = kernel_item.production;
      }
    }
  }
}
} // namespace cyy::computation
