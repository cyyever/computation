/*!
 * \file ll_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "ll_grammar.hpp"

#include <cassert>
#include <iostream>
#include <ranges>

#include "../exception.hpp"

namespace cyy::computation {

  void LL_grammar::construct_parsing_table() const {
    auto follow_sets = follow();
    for (const auto &[head, bodies] : get_productions()) {
      for (auto const &body : bodies) {
        auto const [first_set, epsilon_in_first] = first(body);

        if (epsilon_in_first) {
          auto it = follow_sets.find(head);
          if (it != follow_sets.end()) {
            for (auto const &follow_terminal : it->second) {

              auto [it2, has_inserted] = parsing_table.try_emplace(
                  std::pair{follow_terminal, head}, body);
              // not LL1
              if (!has_inserted) {
                std::cerr << "follow terminal "
                          << alphabet->to_string(follow_terminal);
                std::cerr << " confliction for production:\n";
                CFG_production(it2->first.second, it2->second)
                    .print(std::cerr, *alphabet);
                throw cyy::computation::exception::no_LL_grammar("");
              }
            }
          }
        }
        for (auto const &terminal : first_set) {
          auto [it, has_inserted] =
              parsing_table.try_emplace(std::pair{terminal, head}, body);
          // not LL1
          if (!has_inserted) {

            std::cerr << "first terminal " << alphabet->to_string(terminal);
            std::cerr << " confliction for production:\n";
            CFG_production(it->first.second, it->second)
                .print(std::cerr, *alphabet);
            std::cerr << " and production:\n";
            CFG_production(head, body).print(std::cerr, *alphabet);
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

    if (parsing_table.empty()) {
      construct_parsing_table();
    }
    std::vector<grammar_symbol_type> stack{ALPHABET::endmarker,
                                           get_start_symbol()};
    std::vector<
        std::pair<decltype(this->parsing_table)::const_iterator, size_t>>
        callback_arguments_stack;

    auto endmarked_view = endmarked_symbol_string(view);
    auto terminal_it = endmarked_view.begin();
    while (!stack.empty()) {
      auto top_symbol = std::move(stack.back());
      stack.pop_back();

      auto terminal = *terminal_it;
      if (top_symbol.is_terminal()) {
        auto s = top_symbol.get_terminal();
        if (terminal != s) {
          std::cerr << "symbol does not match terminal:"
                    << alphabet->to_string(terminal) << ' '
                    << alphabet->to_string(s) << std::endl;
          return false;
        }
        terminal_it++;
      } else {
        auto ptr = top_symbol.get_nonterminal_ptr();
        auto it = parsing_table.find({terminal, *ptr});
        if (it == parsing_table.end()) {
          std::cerr << "no rule for parsing " << alphabet->to_string(terminal);
          std::cerr << " for " << *ptr << std::endl;
          return false;
        }

        auto pos = it->second.size();
        for (auto rit = it->second.rbegin(); rit != it->second.rend();
             rit++, pos--) {
          stack.push_back(*rit);
          callback_arguments_stack.emplace_back(it, pos);
        }
        callback_arguments_stack.emplace_back(it, 0);
      }

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
    }
    assert(callback_arguments_stack.empty());
    assert(terminal_it == endmarked_view.end());
    return true;
  }

  CFG::parse_node_ptr
  LL_grammar::get_parse_tree(symbol_string_view view) const {
    auto root = std::make_shared<parse_node>(get_start_symbol());

    std::vector<parse_node_ptr> stack{root};

    if (parse(view, [&stack](auto const &production, const auto pos) {
          if (pos == 0) {
            auto node = std::move(stack.back());
            stack.pop_back();

            for (auto const &grammar_symbol : production.get_body()) {
              node->children.push_back(
                  std::make_shared<parse_node>(grammar_symbol));
            }

            for (auto const &child : node->children | std::ranges::reverse) {
              if (child->grammar_symbol.is_nonterminal()) {
                stack.push_back(child);
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
