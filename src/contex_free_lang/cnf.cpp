/*!
 * \file grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include <range/v3/algorithm.hpp>

#include "cfg.hpp"

namespace cyy::computation {

  void CFG::eliminate_epsilon_productions() {
    auto nullable_nonterminals = nullable();
    if (nullable_nonterminals.empty()) {
      return;
    }

    auto checking_heads = get_heads();
    while (!checking_heads.empty()) {
      auto head = checking_heads.extract(checking_heads.begin()).value();
      auto &bodies = productions[head];

      bodies.erase(ranges::v3::remove_if(bodies,
                                         [](auto const &production) {
                                           return production.empty();
                                         }),
                   bodies.end());

      if (bodies.empty()) {
        productions.erase(head);
        continue;
      }
      std::vector<CFG_production::body_type> new_bodies;
      for (auto &body : bodies) {
        for (auto it = body.begin(); it != body.end(); it++) {
          auto ptr = it->get_nonterminal_ptr();
          if (!ptr || !nullable_nonterminals.count(*ptr)) {
            continue;
          }
          nonterminal_type new_head;
          if (it + 1 != body.end()) {
            new_head = get_new_head(head);
            productions[new_head].emplace_back(std::move_iterator(it + 1),
                                               std::move_iterator(body.end()));
            body.erase(it + 1, body.end());
            body.emplace_back(new_head);
            checking_heads.insert(new_head);
          }

          new_bodies.emplace_back(body.begin(), it);

          if (!new_head.empty()) {
            new_bodies.back().push_back(new_head);
          }
          break;
        }
      }
      if (!new_bodies.empty()) {
        bodies.insert(bodies.end(), new_bodies.begin(), new_bodies.end());
      }
    }
    eliminate_useless_symbols();
    normalize_productions();
  }

  std::set<CFG::nonterminal_type> CFG::nullable() const {
    std::set<CFG::nonterminal_type> nullable_nonterminals;

    bool has_new_nullable_nonterminals = true;
    while (has_new_nullable_nonterminals) {
      has_new_nullable_nonterminals = false;
      for (auto &[head, bodies] : productions) {
        for (auto const &body : bodies) {
          if (ranges::v3::all_of(body, [&](auto const &symbol) {
                return symbol.is_nonterminal() &&
                       nullable_nonterminals.count(
                           *symbol.get_nonterminal_ptr()) != 0;
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

    std::map<nonterminal_type, std::set<nonterminal_type>> single_productions;

    // find immediate single productions
    for (auto &[head, bodies] : productions) {
      auto &this_head = head;
      bodies.erase(ranges::v3::remove_if(
                       bodies,
                       [&this_head, &single_productions](auto const &body) {
                         bool res =
                             body.size() == 1 && body[0].is_nonterminal();
                         if (res && body[0] != this_head) {
                           single_productions[this_head].insert(
                               body[0].get_nonterminal());
                         }
                         return res;
                       }),
                   bodies.end());
    }

    if (single_productions.empty()) {
      return;
    }

    // get all single productions by derivation
    while (true) {
      bool has_new_derivation = false;
      for (auto &[head, derived_nonterminals] : single_productions) {
        for (auto &derived_nonterminal : derived_nonterminals) {
          auto it = single_productions.find(derived_nonterminal);
          if (it == single_productions.end()) {
            continue;
          }
          for (const auto &tmp : it->second) {
            if (head == tmp) {
              continue;
            }
            auto has_insert = derived_nonterminals.insert(tmp).second;
            if (has_insert) {
              has_new_derivation = true;
            }
          }
        }
      }
      if (!has_new_derivation) {
        break;
      }
    }

    auto checking_heads = get_heads();
    while (!checking_heads.empty()) {
      auto head = checking_heads.extract(checking_heads.begin()).value();

      auto &bodies = productions[head];

      std::vector<CFG_production::body_type> new_bodies;

      for (auto &body : bodies) {
        for (auto it = body.begin(); it != body.end(); it++) {
          auto ptr = it->get_nonterminal_ptr();
          if (!ptr) {
            continue;
          }

          auto it2 = single_productions.find(*ptr);
          if (it2 == single_productions.end()) {
            continue;
          }

          nonterminal_type new_head;
          if (it + 1 != body.end()) {
            new_head = get_new_head(head);
            productions[new_head].emplace_back(std::move_iterator(it + 1),
                                               std::move_iterator(body.end()));
            body.erase(it + 1, body.end());
            body.emplace_back(new_head);
            checking_heads.insert(new_head);
          }

          for (auto const &derived_nonterminal : it2->second) {
            new_bodies.emplace_back(body.begin(), it);
            new_bodies.back().push_back(derived_nonterminal);
            if (!new_head.empty()) {
              new_bodies.back().push_back(new_head);
            }
          }
        }
      }
      if (!new_bodies.empty()) {
        bodies.insert(bodies.end(), new_bodies.begin(), new_bodies.end());
      }
    }
    normalize_productions();
  }

  void CFG::to_CNF() {
    eliminate_single_productions();

    std::map<terminal_type, nonterminal_type> terminal_productions;
    auto heads = get_heads();

    while (true) {
      decltype(productions) new_productions;
      for (auto &[head, bodies] : productions) {
        for (auto &body : bodies) {
          if (body.size() == 1) {
            continue;
          }

          for (auto &symbol : body) {
            auto terminal_ptr = symbol.get_terminal_ptr();
            if (!terminal_ptr) {
              continue;
            }
            auto it = terminal_productions.find(*terminal_ptr);
            if (it != terminal_productions.end()) {
              symbol = it->second;
            } else {
              auto new_head = get_new_head("Z", heads);
              heads.insert(new_head);
              terminal_productions[*terminal_ptr] = new_head;
              new_productions[new_head].emplace_back(
                  CFG_production::body_type{*terminal_ptr});
              symbol = new_head;
            }
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
    normalize_productions();
  }

  bool CFG::is_CNF() const {
    for (auto &[_, bodies] : productions) {
      for (auto &body : bodies) {
        if (body.size() == 1) {
          if (!body[0].is_terminal()) {
            return false;
          }
        } else if (body.size() == 2) {

          if (!body[0].is_nonterminal()) {
            return false;
          }
          if (!body[1].is_nonterminal()) {
            return false;
          }
        }
      }
    }
    return true;
  }

} // namespace cyy::computation
