/*!
 * \file grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "cfg.hpp"

namespace cyy::computation {

void CFG::eliminate_epsilon_productions() {
  auto nullable_nonterminals = nullable();

  if (nullable_nonterminals.empty()) {
    return;
  }
  CFG_production::body_type epsilon_production{alphabet->get_epsilon()};

  auto heads = get_heads();
  while (!heads.empty()) {
    auto checking_heads = std::move(heads);
    heads.clear();
    for (auto const &head : checking_heads) {

      std::vector<CFG_production::body_type> new_bodies;
      for (auto &body : productions[head]) {
        if (body.empty()) {
          continue;
        }
        if (body == epsilon_production) {
          body.clear();
          continue;
        }
        for (size_t i = 0; i < body.size(); i++) {
          auto ptr = body[i].get_nonterminal_ptr();
          if (ptr && nullable_nonterminals.count(*ptr)) {
            nonterminal_type new_head;
            if (i + 1 < body.size()) {
              new_head = get_new_head(head);
              productions[new_head].emplace_back(
                  std::move_iterator(body.begin() + i + 1),
                  std::move_iterator(body.end()));
              body.erase(body.begin() + i + 1, body.end());
              body.emplace_back(new_head);
              heads.insert(new_head);
            }

            new_bodies.emplace_back(body.begin(), body.begin() + i);

            if (!new_head.empty()) {
              new_bodies.back().push_back(new_head);
            }

            if (new_bodies.back().empty()) {
              new_bodies.pop_back();
            }
            break;
          }
        }
      }
      if (!new_bodies.empty()) {
        productions[head].insert(productions[head].end(), new_bodies.begin(),
                                 new_bodies.end());
      }
    }
  }
  eliminate_useless_symbols();
  normalize_productions();
}

std::set<CFG::nonterminal_type> CFG::nullable() const {
  std::set<CFG::nonterminal_type> res;
  while (true) {
    bool has_insert = false;

    for (auto &[head, bodies] : productions) {
      if (res.count(head)) {
        continue;
      }
      for (auto const &body : bodies) {
        bool body_nullable = true;
        for (auto const &symbol : body) {
          auto terminal_ptr = symbol.get_terminal_ptr();
          if (terminal_ptr) {
            if (!alphabet->is_epsilon(*terminal_ptr)) {
              body_nullable = false;
              break;
            }
            continue;
          }

          if (res.count(*symbol.get_nonterminal_ptr()) == 0) {
            body_nullable = false;
            break;
          }
        }
        if (body_nullable) {
          res.insert(head);
          has_insert = true;
          break;
        }
      }
    }
    if (!has_insert) {
      break;
    }
  }
  return res;
}

void CFG::eliminate_single_productions() {

  eliminate_epsilon_productions();

  std::map<nonterminal_type, std::set<nonterminal_type>> single_productions;

  // find immediate single productions
  for (auto &[head, bodies] : productions) {
    for (auto &body : bodies) {
      if (body.size() != 1) {
        continue;
      }
      auto ptr = body.front().get_nonterminal_ptr();
      if (!ptr) {
        continue;
      }

      if (*ptr != head) {
        single_productions[head].insert(*ptr);
      }
      body.clear();
    }
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

  auto heads = get_heads();
  while (!heads.empty()) {
    auto checking_heads = std::move(heads);
    heads.clear();
    for (auto const &head : checking_heads) {
      std::vector<CFG_production::body_type> new_bodies;
      for (auto &body : productions[head]) {
        if (body.empty()) {
          continue;
        }
        for (size_t i = 0; i < body.size(); i++) {
          auto ptr = body[i].get_nonterminal_ptr();
          if (!ptr) {
            continue;
          }

          auto it = single_productions.find(*ptr);
          if (it == single_productions.end()) {
            continue;
          }

          nonterminal_type new_head;
          if (i + 1 < body.size()) {
            new_head = get_new_head(head);
            productions[new_head].emplace_back(
                std::move_iterator(body.begin() + i + 1),
                std::move_iterator(body.end()));
            body.erase(body.begin() + i + 1, body.end());
            body.emplace_back(new_head);
            heads.insert(new_head);
          }

          for (auto const &derived_nonterminal : it->second) {
            new_bodies.emplace_back(body.begin(), body.begin() + i);
            new_bodies.back().push_back(derived_nonterminal);
            if (!new_head.empty()) {
              new_bodies.back().push_back(new_head);
            }
          }
        }
      }
      if (!new_bodies.empty()) {
        productions[head].insert(productions[head].end(), new_bodies.begin(),
                                 new_bodies.end());
      }
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
