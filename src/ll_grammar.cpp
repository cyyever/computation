/*!
 * \file grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "ll_grammar.hpp"

namespace cyy::lang {

bool LL_grammar::is_LL1(const std::map<nonterminal_type, std::set<terminal_type>>
                     &nonterminal_first_sets,
                 const std::map<nonterminal_type, std::set<terminal_type>>
                     &follow_sets) const {

  auto has_intersection = [](const auto &set1, const auto &set2) {
    auto it1 = set1.begin();
    auto it2 = set2.begin();
    while (it1 != set1.end() && it2 != set2.end()) {
      if (*it1 < *it2) {
        it1++;
      } else if (*it1 == *it2) {
        return true;
      } else {
        it2++;
      }
    }
    return false;
  };

  for (const auto &[head, bodies] : productions) {

    std::vector<std::set<CFG::terminal_type>> first_sets;
    auto follow_it = follow_sets.find(head);
    for (size_t i = 0; i < bodies.size(); i++) {
      first_sets.emplace_back(
          first({bodies[i].data(), bodies[i].size()}, nonterminal_first_sets));
      for (size_t j = 0; j < i; j++) {
        if (has_intersection(first_sets[i], first_sets[j])) {
          return false;
        }

        if (first_sets[i].count(alphabet->get_epsilon()) != 0 &&
            has_intersection(follow_it->second, first_sets[j])) {
          return false;
        }

        if (first_sets[j].count(alphabet->get_epsilon()) != 0 &&
            has_intersection(follow_it->second, first_sets[i])) {
          return false;
        }
      }
    }
  }
  return true;
}

bool LL_grammar::is_LL1() const {
  auto first_sets = first();
  auto follow_sets = follow(first_sets);

  return is_LL1(first_sets, follow_sets);
}

CFG::parse_node_ptr LL_grammar::parse(symbol_string_view view) const {

  std::map<std::pair<CFG::terminal_type, CFG::nonterminal_type>,
           const production_body_type &>
      parsing_table;

  {
    auto nonterminal_first_sets = first();
    auto follow_sets = follow(nonterminal_first_sets);
    for (const auto &[head, bodies] : productions) {
      for (auto const &body : bodies) {
        auto first_set =
            first({body.data(), body.size()}, nonterminal_first_sets);

        for (auto const &terminal : first_set) {

          if (is_epsilon(terminal)) {
            auto it = follow_sets.find(head);
            if (it != follow_sets.end()) {
              for (auto const &follow_terminal : it->second) {

                auto [it2, has_inserted] = parsing_table.emplace(
                    std::pair{follow_terminal, head}, body);
                // not LL1
                if (!has_inserted) {
                  puts("not LL1 grammar");

                  print(std::cout, head, it2->second);

                  return {};
                }
              }
            }
            continue;
          }
          auto [it, has_inserted] =
              parsing_table.emplace(std::pair{terminal, head}, body);

          // not LL1
          if (!has_inserted) {
            puts("not LL1 grammar");
            print(std::cout, head, it->second);
            puts("confict LL1 grammar");
            print(std::cout, head, body);
            return {};
          }
        }
      }
    }
  }

  puts("begin parse");

  auto tree = std::make_shared<parse_node>(start_symbol);

  std::vector<parse_node_ptr> stack{tree};
  auto endmarker = alphabet->get_endmarker();
  while (!stack.empty()) {
    auto terminal = view.empty() ? endmarker : view.front();
    auto top = std::move(stack.back());
    stack.pop_back();

    if (auto ptr = std::get_if<terminal_type>(&(top->grammar_symbol))) {
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
        {terminal, std::get<nonterminal_type>(top->grammar_symbol)});

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
} // namespace cyy::lang
