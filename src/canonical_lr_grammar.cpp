/*!
 * \file canonical_lr_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "canonical_lr_grammar.hpp"
#include "exception.hpp"

namespace cyy::lang {

LR_1_item_set canonical_LR_grammar::closure(LR_1_item_set set) {
  LR_1_item_set res;
  while (!set.empty()) {
    LR_1_item item = std::move(set.extract(set.begin()).value());

    auto const &[head, body] = item.item.production;
    if (item.item.dot_pos >= body.size()) {
      res.emplace(std::move(item));
      continue;
    }
    auto const &symbol = body[item.item.dot_pos];

    auto ptr = std::get_if<nonterminal_type>(&symbol);

    if (!ptr) {
      res.emplace(std::move(item));
      continue;
    }

    auto lookahead_set =
        first(grammar_symbol_string_view(body.data() + item.item.dot_pos + 1,
                                         body.size() - item.item.dot_pos - 1));
    std::cout << "look ah size is " << lookahead_set.size() << std::endl;

    if (lookahead_set.erase(alphabet->get_epsilon())) {
      lookahead_set.emplace(item.lookahead);
    }

    for (auto a : lookahead_set) {
      std::cout << "look ah is " << a << std::endl;
    }
    auto it = productions.find(*ptr);
    for (auto const &new_body : it->second) {
      LR_1_item new_item;

      if (is_epsilon(new_body[0])) {
        new_item.item.dot_pos = 1;
      } else {
        new_item.item.dot_pos = 0;
      }
      new_item.item.production.first = *ptr;
      new_item.item.production.second = new_body;

      for (auto const &lookahead : lookahead_set) {
        new_item.lookahead = lookahead;
        if (!res.count(new_item)) {
          set.emplace(new_item);
        }
      }
    }
    res.emplace(std::move(item));
  }

  return res;
}

LR_1_item_set canonical_LR_grammar::GOTO(LR_1_item_set set,
                                         const grammar_symbol_type &symbol) {
  LR_1_item_set res;

  while (!set.empty()) {
    LR_1_item item = std::move(set.extract(set.begin()).value());

    if (item.item.dot_pos < item.item.production.second.size() &&
        item.item.production.second[item.item.dot_pos] == symbol) {
      item.item.dot_pos++;
      res.emplace(std::move(item));
    }
  }
  return closure(std::move(res));
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
  init_set.emplace(
      LR_1_item{LR_0_item{production_type{new_start_symbol, {start_symbol}}, 0},
                endmarker});
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

    for (const auto &item : set) {
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
