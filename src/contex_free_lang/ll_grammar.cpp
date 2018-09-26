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
      auto first_set = first({body.data(), body.size()});

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

CFG::parse_node_ptr LL_grammar::parse(symbol_string_view view) const {

  puts("begin parse");

  auto tree = std::make_shared<parse_node>(start_symbol);

  std::vector<parse_node_ptr> stack{tree};
  auto endmarker = alphabet->get_endmarker();
  while (!stack.empty()) {
    auto terminal = view.empty() ? endmarker : view.front();
    auto top = std::move(stack.back());
    stack.pop_back();

    if (auto ptr = top->grammar_symbol.get_terminal_ptr()) {
      if (!is_epsilon(*ptr)) {
        if (terminal != *ptr) {
          puts("terminal is not ptr ");
          alphabet->print(std::cout, terminal);
          alphabet->print(std::cout, *ptr);
          return {};
        }
        view.remove_prefix(1);
      }
      top->grammar_symbol = terminal;
      continue;
    }

    auto it = parsing_table.find(
        {terminal, *(top->grammar_symbol.get_nonterminal_ptr())});

    if (it == parsing_table.end()) {
      puts("no rule for parsing");
      return {};
    }

    for (auto const &grammar_symbol : it->second) {
      top->children.push_back(std::make_shared<parse_node>(grammar_symbol));
    }

    for (auto rit = top->children.rbegin(); rit != top->children.rend();
         rit++) {
      stack.push_back(*rit);
    }
  }

  if (!view.empty()) {
    puts("have symbols remain after parse.");
    return {};
  }

  return tree;
}
} // namespace cyy::computation
