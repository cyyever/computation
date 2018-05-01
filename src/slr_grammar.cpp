/*!
 * \file slr_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "slr_grammar.hpp"
#include "exception.hpp"

namespace cyy::lang {

LR_0_item_set SLR_grammar::closure(LR_0_item_set set) const {

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

  while (has_added) {
    has_added = false;

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
  }

  // process epsilon production
  for (const auto &nonkernel_item : set.nonkernel_items) {
    auto it = productions.find(nonkernel_item);

    for (auto const &body : it->second) {
      auto const &symbol = body[0];
      if (auto ptr = std::get_if<terminal_type>(&symbol); ptr) {
        if (alphabet->is_epsilon(*ptr)) {
          LR_0_item kernel_item;
          kernel_item.production = {it->first, body};
          kernel_item.dot_pos = 1;
          set.kernel_items.emplace(std::move(kernel_item));
        }
      }
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
    if (kernel_item.dot_pos < kernel_item.production.second.size() &&
        kernel_item.production.second[kernel_item.dot_pos] == symbol) {
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
  std::map<std::pair<uint64_t, grammar_symbol_type>, uint64_t> goto_transitions;

  LR_0_item_set init_set;
  init_set.kernel_items.emplace(
      LR_0_item{production_type{new_start_symbol, {start_symbol}}, 0});
  collection.emplace_back(closure(init_set));

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

void SLR_grammar::construct_parsing_table() {
  auto [collection, goto_transitions] = canonical_collection();
  auto first_sets = first();
  auto follow_sets = follow(first_sets);
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

    for (const auto &kernel_item : set.kernel_items) {
      if (kernel_item.dot_pos != kernel_item.production.second.size()) {
        continue;
      }

      if (kernel_item.production.first == new_start_symbol) {
        action_table[{i, endmarker}] = true;
        continue;
      }

      for (auto const &follow_terminal :
           follow_sets[kernel_item.production.first]) {

        // conflict
        if (action_table.count({i, follow_terminal}) != 0) {
	  std::cout<<"config with follow_terminal" <<follow_terminal<<std::endl;
          throw cyy::computation::exception::no_SLR_grammar("");
        }
        action_table[{i, follow_terminal}] = kernel_item.production;
      }
    }
  }
}
} // namespace cyy::lang
