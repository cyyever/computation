/*!
 * \file grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include <cassert>

#include "cfg.hpp"

namespace cyy::computation {

  void CFG::eliminate_epsilon_productions() {
    auto nullable_nonterminals = nullable();
    if (nullable_nonterminals.empty()) {
      return;
    }

    for (auto &[head, bodies] : productions) {
      production_body_set_type new_bodies;
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
          std::ranges::move(split_tmp, std::back_inserter(tmp));
        }
        std::ranges::move(tmp, std::inserter(new_bodies, new_bodies.begin()));
      }
      bodies.merge(std::move(new_bodies));
      std::erase_if(bodies, [](auto const &body) { return body.empty(); });
    }

    if (nullable_nonterminals.contains(start_symbol)) {
      productions[start_symbol].emplace();
    }

    normalize_productions();
    assert(nullable().size() <= 1);
  }

  CFG::nonterminal_set_type CFG::nullable() const {
    nonterminal_set_type nullable_nonterminals;

    bool has_new_nullable_nonterminals = true;
    while (has_new_nullable_nonterminals) {
      has_new_nullable_nonterminals = false;
      for (const auto &[head, bodies] : productions) {
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
        CFG::modify_body_set(bodies, [&](auto &body) {
          if (body.size() != 1 || body[0].is_terminal()) {
            return true;
          }
          auto derived_head = body[0].get_nonterminal();
          deleted_single_productions.emplace(head, derived_head);
          flag = true;
          if (head == derived_head) {
            return false;
          }
          for (auto const &derived_body : productions[derived_head]) {
            if (derived_body.size() == 1 && derived_body[0].is_nonterminal() &&
                deleted_single_productions.contains(
                    {head, derived_body[0].get_nonterminal()})) {
              continue;
            }
            bodies.emplace(derived_body);
          }
          return false;
        });
      }
    }
    normalize_productions();
  }

  void CFG::to_CNF() {
    auto new_start_symbol = get_new_head(start_symbol);
    productions[new_start_symbol] = {{std::move(start_symbol)}};
    start_symbol = std::move(new_start_symbol);
    eliminate_single_productions();

    std::unordered_map<terminal_type, nonterminal_type> terminal_to_nonterminal;

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
        CFG::modify_body_set(bodies,
                             [&](auto &body) {
                               if (body.size() <= 1) {
                                 return true;
                               }

                               for (auto &symbol : body) {
                                 if (!symbol.is_terminal()) {
                                   continue;
                                 }
                                 symbol =
                                     get_terminal_head(symbol.get_terminal());
                               }

                               if (body.size() == 2) {
                                 return true;
                               }

                               auto new_head = get_new_head(head, heads);
                               heads.insert(new_head);

                               new_productions[new_head].emplace(
                                   std::move_iterator(body.begin() + 1),
                                   std::move_iterator(body.end()));
                               body.resize(1);
                               body.emplace_back(new_head);
                               return true;
                             }

        );
      }

      if (!new_productions.empty()) {
        productions.merge(new_productions);
      } else {
        break;
      }
    }
    for (auto const &[terminal, head] : terminal_to_nonterminal) {
      productions[head].emplace(CFG_production::body_type{terminal});
    }
    normalize_productions();
  }

} // namespace cyy::computation
