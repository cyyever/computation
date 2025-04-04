/*!
 * \file ll_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

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
      const std::function<void(const CFG_production &, size_t pos)>
          &match_callback) const {

    if (parsing_table.empty()) {
      construct_parsing_table();
    }
    std::vector<grammar_symbol_type> stack{ALPHABET::endmarker,
                                           get_start_symbol()};
    auto endmarked_view = cyy::algorithm::endmarked_symbol_string(view);
    auto terminal_it = endmarked_view.begin();
    while (!stack.empty() && terminal_it != endmarked_view.end()) {
      auto top_symbol = std::move(stack.back());
      stack.pop_back();

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

      auto const &head = it->first.second;
      auto const &body = it->second;
      for (auto symbol : body | std::views::reverse) {
        stack.emplace_back(std::move(symbol));
      }

      for (std::size_t pos = 0; pos <= body.size() + 1; pos++) {

        match_callback({head, body}, pos);
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
