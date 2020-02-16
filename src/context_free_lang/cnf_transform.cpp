/*!
 * \file grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include <cassert>
#include <range/v3/algorithm.hpp>

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
        if (!ranges::any_of(body.get_nonterminal_view(),
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

    /*
    auto checking_heads = get_heads();
    while (!checking_heads.empty()) {
      auto head = checking_heads.extract(checking_heads.begin()).value();
      auto &bodies = productions[head];

      bodies.erase(ranges::remove_if(bodies,
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
          if (it->is_terminal() ||
              !nullable_nonterminals.contains(it->get_nonterminal())) {
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

          if (body.begin() != it) {
            new_bodies.emplace_back(body.begin(), it);

            if (!new_head.empty()) {
              new_bodies.back().push_back(new_head);
            }
          }
          break;
        }
      }
      if (!new_bodies.empty()) {
        bodies.insert(bodies.end(), new_bodies.begin(), new_bodies.end());
      }
    }
    eliminate_useless_symbols();
    */

    normalize_productions();
    assert(nullable().size() <= 1);
  }

  std::set<CFG::nonterminal_type> CFG::nullable() const {
    std::set<CFG::nonterminal_type> nullable_nonterminals;

    bool has_new_nullable_nonterminals = true;
    while (has_new_nullable_nonterminals) {
      has_new_nullable_nonterminals = false;
      for (auto &[head, bodies] : productions) {
        for (auto const &body : bodies) {
          if (ranges::all_of(body, [&](auto const &symbol) {
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
        std::cout<<"check head"<<head<<std::endl;
        for (size_t i = 0; i < bodies.size();) {
        std::cout<<"i is "<<i<<std::endl;
          if (bodies[i].size() != 1 || bodies[i][0].is_nonterminal()) {
            i++;
            continue;
          }
          auto derived_head = bodies[i][0].get_nonterminal();
          deleted_single_productions.emplace(head, derived_head);
          std::swap(bodies[i], *bodies.rbegin());
          bodies.pop_back();
          flag = true;
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

    /*
    // find immediate single productions
    for (auto &[head, bodies] : productions) {
      auto &this_head = head;
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
    */
    normalize_productions();
  }

  void CFG::to_CNF() {
    auto new_start_symbol = get_new_head(start_symbol);
    productions[new_start_symbol] = {{start_symbol}};
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
            if (!symbol.is_terminal()) {
              continue;
            }
            auto s = symbol.get_terminal();
            auto it = terminal_productions.find(s);
            if (it != terminal_productions.end()) {
              symbol = it->second;
            } else {
              auto new_head = get_new_head("Z", heads);
              heads.insert(new_head);
              terminal_productions[s] = new_head;
              new_productions[new_head].emplace_back(
                  CFG_production::body_type{s});
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
        } else {
          return false;
        }
      }
    }
    return true;
  }

} // namespace cyy::computation
