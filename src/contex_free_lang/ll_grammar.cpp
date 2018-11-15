/*!
 * \file ll_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "ll_grammar.hpp"
#include "../exception.hpp"

namespace cyy::computation {

void LL_grammar::construct_parsing_table() {
  auto follow_sets = follow();
  for (const auto &[head, bodies] : productions) {
    for (auto const &body : bodies) {
      auto first_set = first({body});

      for (auto const &terminal : first_set) {

        if (is_epsilon(terminal)) {
          auto it = follow_sets.find(head);
          if (it != follow_sets.end()) {
            for (auto const &follow_terminal : it->second) {

              auto has_inserted =
                  parsing_table.emplace(std::pair{follow_terminal, head}, body)
                      .second;
              // not LL1
              if (!has_inserted) {
                throw cyy::computation::exception::no_LL_grammar("");
              }
            }
          }
          continue;
        }
        auto has_inserted =
            parsing_table.emplace(std::pair{terminal, head}, body).second;

        // not LL1
        if (!has_inserted) {
          throw cyy::computation::exception::no_LL_grammar("");
        }
      }
    }
  }
}
bool LL_grammar::parse(
    symbol_string_view view,
    const std::function<void(const nonterminal_type &,
                             const production_body_type &)>
        &match_nonterminal_callback,
    const std::function<void(terminal_type)> &match_terminal_callback) const {

  std::vector<grammar_symbol_type> stack{start_symbol};
  const auto endmarker = alphabet->get_endmarker();
  while (!stack.empty()) {
    const auto terminal = view.empty() ? endmarker : view.front();
    auto top_symbol = std::move(stack.back());
    stack.pop_back();

    if (auto ptr = top_symbol.get_terminal_ptr()) {
      if (!is_epsilon(*ptr)) {
        if (terminal != *ptr) {
          std::cerr << "symbol does not match terminal:";
          alphabet->print(std::cerr, terminal);
          alphabet->print(std::cerr, *ptr);
          std::cout << std::endl;
          return false;
        }
        view.remove_prefix(1);
      }
      match_terminal_callback(*ptr);
      continue;
    }

    auto ptr = top_symbol.get_nonterminal_ptr();
    auto it = parsing_table.find({terminal, *ptr});

    if (it == parsing_table.end()) {
      std::cerr << "no rule for parsing";
      alphabet->print(std::cerr, terminal);
      std::cerr << ' ' << *ptr;
      std::cerr << std::endl;
      return false;
    }

    for (auto rit = it->second.rbegin(); rit != it->second.rend(); rit++) {
      stack.push_back(*rit);
    }
    match_nonterminal_callback(*ptr, it->second);
  }

  if (!view.empty()) {
    std::cerr << "have symbols remain after parse:";
    for (auto const &terminal : view) {
      print(std::cerr, terminal);
    }
    std::cerr << std::endl;
    return false;
  }

  return true;
}

CFG::parse_node_ptr LL_grammar::get_parse_tree(symbol_string_view view) const {
  auto root = std::make_shared<parse_node>(start_symbol);

  std::vector<parse_node_ptr> stack{root};

  if (parse(view,

            [&stack](auto const &head, auto const &body) {
              auto node = std::move(stack.back());
              stack.pop_back();

              for (auto const &grammar_symbol : body) {
                node->children.push_back(
                    std::make_shared<parse_node>(grammar_symbol));
              }

              for (auto rit = node->children.rbegin();
                   rit != node->children.rend(); rit++) {
                stack.push_back(*rit);
              }
            },
            [&stack](auto terminal) { stack.pop_back(); })) {
    return root;
  }
  return {};
}
} // namespace cyy::computation
