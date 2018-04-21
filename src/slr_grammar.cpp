/*!
 * \file slr_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "slr_grammar.hpp"

namespace cyy::lang {

LR_0_item_set SLR_grammar::closure(LR_0_item_set set) const {
  while (true) {
    bool has_added = false;
    for (const auto &kernel_item : set.kernel_items) {
      if (kernel_item.dot_pos < kernel_item.production.second.size()) {
        auto const &symbol = kernel_item.production.second[kernel_item.dot_pos];
        if (!std::holds_alternative<nonterminal_type>(symbol)) {
          continue;
        }
        if (set.nonkernel_items.insert(std::get<nonterminal_type>(symbol))
                .second) {
          has_added = true;
        }
      }
    }

    for (const auto &nonkernel_item : set.nonkernel_items) {
      auto it = productions.find(nonkernel_item);

      for (auto const &body : it->second) {
        auto const &symbol = body[0];
        if (!std::holds_alternative<nonterminal_type>(symbol)) {
          continue;
        }
        if (set.nonkernel_items.insert(std::get<nonterminal_type>(symbol))
                .second) {
          has_added = true;
        }
      }
    }
    if (!has_added) {
      break;
    }
  }
  return set;
}

LR_0_item_set SLR_grammar::GOTO(LR_0_item_set set,
                                const grammar_symbol_type &symbol) const {
  LR_0_item_set res;
  while (!set.kernel_items.empty()) {
    LR_0_item kernel_item =
        std::move(set.kernel_items.extract(set.kernel_items.begin()).value());
    if (kernel_item.dot_pos < kernel_item.production.second.size()) {
      if (kernel_item.production.second[kernel_item.dot_pos] != symbol) {
        continue;
      }
      kernel_item.dot_pos++;
      res.kernel_items.emplace(std::move(kernel_item));
    }
  }

  while (!set.nonkernel_items.empty()) {
    nonterminal_type nonkernel_item = std::move(
        set.nonkernel_items.extract(set.nonkernel_items.begin()).value());
    auto it = productions.find(nonkernel_item);

    for (auto const &body : it->second) {
      if (body[0] == symbol) {

        LR_0_item new_kernel_item;
        new_kernel_item.production.first = nonkernel_item;
        new_kernel_item.production.second = body;
        new_kernel_item.dot_pos = 1;
        res.kernel_items.emplace(std::move(new_kernel_item));
      }
    }
  }

  return closure(std::move(res));
}

std::pair<
    std::vector<LR_0_item_set>,
    std::map<std::pair<uint64_t, SLR_grammar::grammar_symbol_type>, uint64_t>>
SLR_grammar::canonical_collection() const {
  std::vector<LR_0_item_set> collection;
  std::vector<bool> check_flag{true};
  std::map<std::pair<uint64_t, grammar_symbol_type>, uint64_t> goto_table;

  LR_0_item_set init_set;
  auto new_start_symbol = get_new_head(start_symbol);
  init_set.kernel_items.emplace(
      LR_0_item{production_type{new_start_symbol, {start_symbol}}, 0});
  collection.emplace_back(std::move(init_set));

  auto terminals = get_terminals();
  auto nonterminals = get_heads();

  uint64_t next_state = 1;
  for (uint64_t i = 0; i < check_flag.size(); i++) {
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
        goto_table[{i, {terminal}}] = next_state;
        next_state++;
      } else {
        goto_table[{i, {terminal}}] = it - collection.begin();
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
        goto_table[{i, {nonterminal}}] = next_state;
        next_state++;
      } else {
        goto_table[{i, {nonterminal}}] = it - collection.begin();
      }
    }
    check_flag[i] = false;
  }

  return {collection, goto_table};
}
} // namespace cyy::lang
