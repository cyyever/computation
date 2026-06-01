/*!
 * \file ll_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include <cassert>
#include <ranges>
#include <utility>
#include <variant>

#include "ll_grammar.hpp"

#include "exception.hpp"

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
                std::cerr << std::format(
                    "follow terminal {} confliction for production:\n {}",
                    alphabet->to_string(follow_terminal),
                    CFG_production(it2->first.second, it2->second)
                        .to_string(*alphabet));
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
            std::cerr << std::format(
                "first terminal {} confliction for production:\n{}\n and "
                "production:\n{}",
                alphabet->to_string(terminal),
                CFG_production(it->first.second, it->second)
                    .to_string(*alphabet),
                CFG_production(head, body).to_string(*alphabet));
            throw cyy::computation::exception::no_LL_grammar("");
          }
        }
      }
    }
  }

  bool LL_grammar::parse(
      symbol_string_view view,
      const std::function<void(const CFG_production &, std::size_t pos)>
          &match_callback) const {

    if (parsing_table.empty()) {
      construct_parsing_table();
    }
    using callback_argument_type =
        std::pair<decltype(this->parsing_table)::const_iterator, std::size_t>;
    // A single stack holds both the grammar symbols still to be matched and the
    // pending match_callback invocations. Interleaving them ensures each
    // (production, pos) callback fires only once the symbol at that position has
    // been fully parsed -- the property 307e984 lost by firing every position
    // up front, before the child symbols were parsed.
    std::vector<std::variant<grammar_symbol_type, callback_argument_type>> stack;
    stack.emplace_back(grammar_symbol_type(ALPHABET::endmarker));
    stack.emplace_back(grammar_symbol_type(get_start_symbol()));

    auto endmarked_view = cyy::algorithm::endmarked_symbol_string(view);
    auto terminal_it = endmarked_view.begin();
    while (!stack.empty()) {
      auto top = std::move(stack.back());
      stack.pop_back();

      if (auto *argument = std::get_if<callback_argument_type>(&top)) {
        auto const &[it, pos] = *argument;
        match_callback({it->first.second, it->second}, pos);
        continue;
      }

      auto const &top_symbol = std::get<grammar_symbol_type>(top);
      auto terminal = *terminal_it;
      if (top_symbol.is_terminal()) {
        const auto s = top_symbol.get_terminal();
        if (terminal != s) {
          std::cerr << std::format("symbol does not match terminal:{} {}\n",
                                   alphabet->to_string(terminal),
                                   alphabet->to_string(s));
          return false;
        }
        terminal_it++;
        continue;
      }

      auto nonterminal = top_symbol.get_nonterminal();
      auto it = parsing_table.find({terminal, nonterminal});
      if (it == parsing_table.end()) {
        std::cerr << std::format("no rule for parsing {} for {} \n",
                                 alphabet->to_string(terminal), nonterminal);
        return false;
      }

      // Push bottom-up so callback(pos=0) ends up on top, each body symbol
      // trailed by the callback for its position:
      //   callback(n), X_n, callback(n-1), ..., X_1, callback(0)
      auto const &body = it->second;
      stack.emplace_back(callback_argument_type{it, body.size()});
      for (auto const &[index, symbol] :
           body | std::views::enumerate | std::views::reverse) {
        stack.emplace_back(symbol);
        stack.emplace_back(
            callback_argument_type{it, static_cast<std::size_t>(index)});
      }
    }
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

            for (auto const &child : node->children | std::views::reverse) {
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
