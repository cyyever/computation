/*!
 * \file grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include <algorithm>
#include <cassert>

#include "cfg.hpp"

namespace cyy::computation {

  void CFG::eliminate_epsilon_productions() {
    auto nullable_nonterminals = nullable();
    if (nullable_nonterminals.empty()) {
      return;
    }

    for (auto &[head, bodies] : productions) {
      std::set<CFG_production::body_type> new_bodies;
      for (auto const &body : bodies) {
        if (body.empty()) {
          continue;
        }
        if (!std::ranges::any_of(body.get_nonterminal_view(),
                                 [&nullable_nonterminals](auto const &g) {
                                   return nullable_nonterminals.contains(g);
                                 })) {
          continue;
        }
        std::vector<CFG_production::body_type> tmp;
        for (auto const &s : body) {
          if (s.is_terminal() ||
              !nullable_nonterminals.contains(s.get_nonterminal())) {
            if (tmp.empty()) {
              tmp.emplace_back(CFG_production::body_type{s});
            } else {
              for (auto &partial_body : tmp) {
                partial_body.emplace_back(s);
              }
            }
            continue;
          }
          auto split_tmp = tmp;
          for (auto &partial_body : tmp) {
            partial_body.emplace_back(s);
          }
          tmp.insert(tmp.end(), std::move_iterator(split_tmp.begin()),
                     std::move_iterator(split_tmp.end()));
        }
        new_bodies.insert(std::move_iterator(tmp.begin()),
                          std::move_iterator(tmp.end()));
      }
      new_bodies.insert(std::move_iterator(bodies.begin()),
                        std::move_iterator(bodies.end()));
      std::erase_if(new_bodies, [](auto const &body) { return body.empty(); });

      bodies = {std::move_iterator(new_bodies.begin()),
                std::move_iterator(new_bodies.end())};
    }

    if (nullable_nonterminals.contains(start_symbol)) {
      productions[start_symbol].emplace_back();
    }

    normalize_productions();
    assert(nullable().size() <= 1);
  }

  std::unordered_set<CFG::nonterminal_type> CFG::nullable() const {
    std::unordered_set<CFG::nonterminal_type> nullable_nonterminals;

    bool has_new_nullable_nonterminals = true;
    while (has_new_nullable_nonterminals) {
      has_new_nullable_nonterminals = false;
      for (auto &[head, bodies] : productions) {
        for (auto const &body : bodies) {
          if (std::ranges::all_of(body, [&](auto const &symbol) {
                return symbol.is_nonterminal() &&
                       nullable_nonterminals.contains(
                           *symbol.get_nonterminal_ptr());
              })) {
            has_new_nullable_nonterminals =
                nullable_nonterminals.emplace(head).second;
          }
        }
      }
    }
    return nullable_nonterminals;
  }

  void CFG::eliminate_single_productions() {
    eliminate_epsilon_productions();

    std::set<std::pair<nonterminal_type, nonterminal_type>>
        deleted_single_productions;

    bool flag = true;
    while (flag) {
      flag = false;
      for (auto &[head, bodies] : productions) {
        for (size_t i = 0; i < bodies.size();) {
          if (bodies[i].size() != 1 || bodies[i][0].is_terminal()) {
            i++;
            continue;
          }
          auto derived_head = bodies[i][0].get_nonterminal();
          deleted_single_productions.emplace(head, derived_head);
          if (i + 1 < bodies.size()) {
            std::swap(bodies[i], *bodies.rbegin());
          }
          bodies.pop_back();
          flag = true;
          if (head == derived_head) {
            continue;
          }
          for (auto const &derived_body : productions[derived_head]) {
            if (derived_body.size() == 1 && derived_body[0].is_nonterminal() &&
                deleted_single_productions.contains(
                    {head, derived_body[0].get_nonterminal()})) {
              continue;
            }
            bodies.push_back(derived_body);
          }
        }
      }
    }
    normalize_productions();
  }

  void CFG::to_CNF() {
    auto new_start_symbol = get_new_head(start_symbol);
    productions[new_start_symbol] = {{start_symbol}};
    start_symbol = new_start_symbol;
    eliminate_single_productions();

    std::map<terminal_type, nonterminal_type> terminal_to_nonterminal;

    auto heads = get_heads();
    auto get_terminal_head = [&terminal_to_nonterminal,
                              &heads](terminal_type s) {
      auto it = terminal_to_nonterminal.find(s);
      if (it != terminal_to_nonterminal.end()) {
        return it->second;
      }
      auto new_head = get_new_head("T_" + std::to_string(s), heads);
      heads.insert(new_head);
      terminal_to_nonterminal[s] = new_head;
      return new_head;
    };

    while (true) {
      decltype(productions) new_productions;
      for (auto &[head, bodies] : productions) {
        for (auto &body : bodies) {
          if (body.size() <= 1) {
            continue;
          }

          for (auto &symbol : body) {
            if (!symbol.is_terminal()) {
              continue;
            }
            auto s = symbol.get_terminal();
            symbol = get_terminal_head(s);
          }

          if (body.size() == 2) {
            continue;
          }

          auto new_head = get_new_head(head, heads);
          heads.insert(new_head);

          new_productions[new_head].emplace_back(
              std::move_iterator(body.begin() + 1),
              std::move_iterator(body.end()));
          body.resize(1);
          body.emplace_back(new_head);
        }
      }

      if (!new_productions.empty()) {
        productions.merge(new_productions);
      } else {
        break;
      }
    }
    for (auto const &[terminal, head] : terminal_to_nonterminal) {
      productions[head].emplace_back(CFG_production::body_type{terminal});
    }
    normalize_productions();
  }

} // namespace cyy::computation
