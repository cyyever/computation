/*!
 * \file ll_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include <cassert>

#include "../exception.hpp"
#include "ll_grammar.hpp"

namespace cyy::computation {

  void LL_grammar::construct_parsing_table() {
    auto follow_sets = follow();
    for (const auto &[head, bodies] : productions) {
      for (auto const &body : bodies) {
        auto first_set = first({body});

        for (auto const &terminal : first_set) {
          if (alphabet->is_epsilon(terminal)) {
            auto it = follow_sets.find(head);
            if (it != follow_sets.end()) {
              for (auto const &follow_terminal : it->second) {

                auto [it2, has_inserted] = parsing_table.emplace(
                    std::pair{follow_terminal, head}, body);
                // not LL1
                if (!has_inserted) {
                  std::cerr << "head and terminal confliction:" << head << ' ';
                  alphabet->print(std::cerr, follow_terminal);
                  std::cerr << ' ';
                  alphabet->print(std::cerr, it2->first.first);
                  std::cerr << std::endl;
                  throw cyy::computation::exception::no_LL_grammar("");
                }
              }
            }
            continue;
          }

          auto [it, has_inserted] =
              parsing_table.emplace(std::pair{terminal, head}, body);
          // not LL1
          if (!has_inserted) {
            std::cerr << "head and terminal confliction:" << head << ' ';
            alphabet->print(std::cerr, terminal);
            std::cerr << ' ';
            alphabet->print(std::cerr, it->first.first);
            std::cerr << std::endl;
            throw cyy::computation::exception::no_LL_grammar("");
          }
        }
      }
    }
  }

  bool LL_grammar::parse(
      symbol_string_view view,
      const std::function<void(const CFG_production &, size_t pos)>
          &match_callback) const {
    std::vector<grammar_symbol_type> stack{start_symbol};
    std::vector<
        std::pair<decltype(this->parsing_table)::const_iterator, size_t>>
        callback_arguments_stack;
    const auto endmarker = alphabet->get_endmarker();
    while (!stack.empty()) {
      const auto terminal = view.empty() ? endmarker : view.front();
      auto top_symbol = std::move(stack.back());
      stack.pop_back();

      if (auto ptr = top_symbol.get_terminal_ptr()) {
        if (!alphabet->is_epsilon(*ptr)) {
          if (terminal != *ptr) {
            std::cerr << "symbol does not match terminal:";
            alphabet->print(std::cerr, terminal);
            alphabet->print(std::cerr, *ptr);
            std::cout << std::endl;
            return false;
          }
          view.remove_prefix(1);
        }

        assert(!callback_arguments_stack.empty());
        while (!callback_arguments_stack.empty()) {
          auto const &[it, pos] = callback_arguments_stack.back();
          auto const &head = it->first.second;
          auto const &body = it->second;
          match_callback({head, body}, pos);
          bool finish_production = (pos == body.size());
          callback_arguments_stack.pop_back();
          if (!finish_production) {
            break;
          }
        }
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

      match_callback(CFG_production{*ptr, it->second}, 0);
      auto pos = it->second.size();
      for (auto rit = it->second.rbegin(); rit != it->second.rend();
           rit++, pos--) {
        stack.push_back(*rit);
        callback_arguments_stack.emplace_back(it, pos);
      }
    }

    if (!view.empty()) {
      std::cerr << "there are symbols remain after parse:";
      for (auto const &terminal : view) {
        alphabet->print(std::cerr, terminal);
      }
      std::cerr << std::endl;
      return false;
    }

    return true;
  }

  CFG::parse_node_ptr
  LL_grammar::get_parse_tree(symbol_string_view view) const {
    auto root = std::make_shared<parse_node>(start_symbol);

    std::vector<parse_node_ptr> stack{root};

    if (parse(view, [&stack](auto const &production, const auto pos) {
          if (pos == 0) {
            auto node = std::move(stack.back());
            stack.pop_back();

            for (auto const &grammar_symbol : production.get_body()) {
              node->children.push_back(
                  std::make_shared<parse_node>(grammar_symbol));
            }

            for (auto rit = node->children.rbegin();
                 rit != node->children.rend(); rit++) {
              if ((*rit)->grammar_symbol.is_nonterminal()) {
                stack.push_back(*rit);
              }
            }
            return;
          }
        })) {
      assert(stack.empty());
      return root;
    }
    return {};
  }
} // namespace cyy::computation
