/*!
 * \file canonical_lr_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "canonical_lr_grammar.hpp"
#include "exception.hpp"

namespace cyy::lang {

LR_1_item_set_ canonical_LR_grammar::GOTO(const LR_1_item_set_  &set,
                                         const grammar_symbol_type &symbol) {
  LR_1_item_set_ res;
  for (auto const & kernel_item : set.get_kernel_items()) {
    if (kernel_item.item.dot_pos < kernel_item.item.production.second.size() &&
        kernel_item.item.production.second[kernel_item.item.dot_pos] == symbol) {
      auto new_kernel_item = kernel_item;
      new_kernel_item.item.dot_pos++;
      res.add_kernel_item(*this, std::move(new_kernel_item));
    }
  }

  for (auto const & [nonterminal,lookahead_set ]  : set.get_nonkernel_items()) {
    auto it = productions.find(nonterminal);

    for (auto const &body : it->second) {
      if (body[0] == symbol) {

    std::cout << "look ah size is " << lookahead_set.size() << std::endl;
	for(auto const &lookahead:lookahead_set) {
        LR_1_item new_kernel_item;
        new_kernel_item.item.production.first =nonterminal;
        new_kernel_item.item.production.second = body;
        new_kernel_item.item.dot_pos = 1;
        new_kernel_item.lookahead= lookahead;
        res.add_kernel_item(*this, std::move(new_kernel_item));
	}
      }
    }
  }
  return res;
}


std::pair<
    std::vector<LR_1_item_set_>,
    std::map<std::pair<uint64_t, canonical_LR_grammar::grammar_symbol_type>,
             uint64_t>>
canonical_LR_grammar::canonical_collection() {
  std::vector<LR_1_item_set_> collection;
  std::vector<bool> check_flag{true};
  std::map<std::pair<uint64_t, grammar_symbol_type>, uint64_t> goto_transitions;
  auto endmarker = alphabet->get_endmarker();

  LR_1_item_set_ init_set;
  init_set.add_kernel_item(*this,
      LR_1_item{LR_0_item{production_type{new_start_symbol, {start_symbol}}, 0},
                endmarker});
  std::cout << "nonkernel_items size is " << init_set.get_nonkernel_items().size() << std::endl;
  collection.emplace_back(init_set);

  auto terminals = get_terminals();
  auto nonterminals = get_heads();

  uint64_t next_state = 1;
  for (size_t i = 0; i < check_flag.size(); i++) {
    if (!check_flag[i]) {
      continue;
    }
    for (auto const &terminal : terminals) {
      auto goto_set = GOTO(collection[i], terminal);

      if (goto_set.empty()) {
        continue;
      }

      auto it = std::find(collection.begin(), collection.end(), goto_set);
      if (it == collection.end()) {
        collection.emplace_back(std::move(goto_set));
        check_flag.emplace_back(true);
        goto_transitions[{i, {terminal}}] = next_state;
        next_state++;
      } else {
        goto_transitions[{i, {terminal}}] = it - collection.begin();
      }
    }

    for (auto const &nonterminal : nonterminals) {
      auto goto_set = GOTO(collection[i], nonterminal);

      if (goto_set.empty()) {
        continue;
      }

      auto it = std::find(collection.begin(), collection.end(), goto_set);
      if (it == collection.end()) {
        collection.emplace_back(std::move(goto_set));
        check_flag.emplace_back(true);
        goto_transitions[{i, {nonterminal}}] = next_state;
        next_state++;
      } else {
        goto_transitions[{i, {nonterminal}}] = it - collection.begin();
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

    for (const auto &item : set.get_kernel_items()) {
      if (item.item.dot_pos != item.item.production.second.size()) {
        continue;
      }

      if (item.item.production.first == new_start_symbol) {
        action_table[{i, endmarker}] = true;
        continue;
      }

      // conflict
      if (action_table.count({i, item.lookahead}) != 0) {
        std::cout << "config with follow_terminal" << item.lookahead
                  << std::endl;
        throw cyy::computation::exception::no_canonical_LR_grammar("");
      }
      action_table[{i, item.lookahead}] = item.item.production;
    }
  }
}
} // namespace cyy::lang
