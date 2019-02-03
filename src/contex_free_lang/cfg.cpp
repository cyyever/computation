/*!
 * \file grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "cfg.hpp"

#include <unordered_map>
#include <utility>

namespace cyy::computation {

CFG::CFG(
    const std::string &alphabet_name, nonterminal_type start_symbol_,
    std::map<nonterminal_type, std::vector<production_body_type>> productions_)
    : alphabet(ALPHABET::get(alphabet_name)),
      start_symbol(std::move(start_symbol_)),
      productions(std::move(productions_)) {

  eliminate_useless_symbols();
  normalize_productions();

  bool has_start_symbol = false;
  for (auto &[head, bodies] : productions) {
    if (!has_start_symbol && head == start_symbol) {
      has_start_symbol = true;
    }

    if (bodies.empty()) {
      throw std::invalid_argument(std::string("no body for head ") + head);
    }

    for (auto const &body : bodies) {
      if (body.empty()) {
        throw std::invalid_argument(std::string("an empty body for head ") +
                                    head);
      }
      for (auto const &symbol : body) {
        auto terminal_ptr = symbol.get_terminal_ptr();
        if (terminal_ptr && !alphabet->is_epsilon(*terminal_ptr) &&
            !alphabet->contain(*terminal_ptr)) {
          throw std::invalid_argument(std::string("invalid terminal ") +
                                      std::to_string(*terminal_ptr));
        }
      }
    }
  }
  if (!has_start_symbol) {
    throw std::invalid_argument("no productions for start symbol");
  }
}

bool CFG::operator==(const CFG &rhs) const {
  return (this == &rhs) ||
         (alphabet->get_name() == rhs.alphabet->get_name() &&
          start_symbol == rhs.start_symbol && productions == rhs.productions);
}

void CFG::print(std::ostream &os) const {
  // by convention,we print start symbol first.
  auto it = productions.find(start_symbol);
  for (auto const &body : it->second) {
    print(os, start_symbol, body);
  }
  for (auto const &[head, bodies] : productions) {
    if (head == start_symbol) {
      continue;
    }
    for (auto const &body : bodies) {
      print(os, head, body);
    }
  }
}

std::set<CFG::nonterminal_type> CFG::get_heads() const {
  std::set<nonterminal_type> heads;
  for (auto const &[head, _] : productions) {
    heads.insert(head);
  }
  return heads;
}

bool CFG::has_production(const production_type &production) const {

  auto it = productions.find(production.first);
  return it != productions.end() &&
         std::find(it->second.begin(), it->second.end(), production.second) !=
             it->second.end();
}

void CFG::normalize_productions() {
  decltype(productions) new_productions;
  for (auto &[head, bodies] : productions) {
    std::set<production_body_type> bodies_set;
    for (auto &body : bodies) {
      if (body.empty()) {
        continue;
      }

      auto it = std::remove_if(body.begin(), body.end(),
                               [this](const auto &grammal_symbol) {
                                 return is_epsilon(grammal_symbol);
                               });
      if (it > body.begin()) {
        bodies_set.emplace(std::move_iterator(body.begin()),
                           std::move_iterator(it));
      } else {
        bodies_set.emplace(1, symbol_type(alphabet->get_epsilon()));
      }
    }
    if (!bodies_set.empty()) {
      new_productions[head] = {std::move_iterator(bodies_set.begin()),
                               std::move_iterator(bodies_set.end())};
    }
  }
  productions = std::move(new_productions);
}

void CFG::eliminate_useless_symbols() {
  if (productions.empty()) {
    return;
  }

  enum class nonterminal_state { unchecked, checking, useless, non_useless };

  std::map<nonterminal_type, nonterminal_state> states;
  std::map<nonterminal_type, std::set<nonterminal_type>> depedency_heads;

  for (const auto &[head, _] : productions) {
    states[head] = nonterminal_state::unchecked;
  }

  auto check_nonterminal = [&](auto &&self, const nonterminal_type &head,
                               CFG &cfg) -> void {
    states[head] = nonterminal_state::checking;

    auto &bodies = cfg.productions[head];
    for (auto &body : bodies) {
      bool useless = false;
      for (auto const &symbol : body) {
        if (!symbol.is_nonterminal()) {
          continue;
        }
        auto nonterminal = *symbol.get_nonterminal_ptr();
        auto it = states.find(nonterminal);
        if (it == states.end()) {
          body.clear();
          useless = true;
          break;
        }
        if (it->second == nonterminal_state::unchecked) {
          self(self, nonterminal, cfg);
          it = states.find(nonterminal);
        }
        if (it->second == nonterminal_state::useless) {
          body.clear();
          useless = true;
          break;
        }
        if (it->second == nonterminal_state::checking) {
          depedency_heads[nonterminal].insert(head);
          useless = true;
        }
      }
      if (!useless) {
        states[head] = nonterminal_state::non_useless;
      }
    }
    bodies.erase(std::remove_if(bodies.begin(), bodies.end(),
                                [](const auto &body) { return body.empty(); }),
                 bodies.end());
    if (bodies.empty()) {
      states[head] = nonterminal_state::useless;
    }

    if (states[head] != nonterminal_state::checking) {
      for (auto const &depedency_head : depedency_heads[head]) {
        if (states[depedency_head] == nonterminal_state::checking) {
          states[depedency_head] = nonterminal_state::unchecked;
          self(self, depedency_head, cfg);
        }
      }
    }
  };

  decltype(productions) new_productions;
  check_nonterminal(check_nonterminal, start_symbol, *this);

  auto add_nonterminal = [&](auto &&self, const nonterminal_type &head,
                             CFG &cfg) -> void {
    auto &bodies = cfg.productions[head];
    for (auto &body : bodies) {
      if (body.empty()) {
        continue;
      }
      bool add = true;
      std::set<nonterminal_type> nonterminals;
      for (auto const &symbol : body) {
        if (!symbol.is_nonterminal()) {
          continue;
        }
        const auto &nonterminal = *(symbol.get_nonterminal_ptr());
        if (states[nonterminal] != nonterminal_state::non_useless) {
          add = false;
          break;
        }
        nonterminals.insert(nonterminal);
      }
      if (!add) {
        continue;
      }
      new_productions[head].emplace_back(std::move(body));
      body.clear();
      for (auto const &nonterminal : nonterminals) {
        self(self, nonterminal, cfg);
      }
    }
  };

  add_nonterminal(add_nonterminal, start_symbol, *this);
  productions = std::move(new_productions);
}

void CFG::eliminate_left_recursion(std::vector<nonterminal_type> old_heads) {

  if (old_heads.empty()) {
    for (const auto &[head, _] : productions) {
      old_heads.push_back(head);
    }
  }

  auto const eliminate_immediate_left_recursion =
      [this](const nonterminal_type &head) {
        auto new_head = get_new_head(head);
        std::vector<production_body_type> new_bodies;
        auto &bodies = productions[head];

        for (auto &body : bodies) {

          if (body.front().is_nonterminal() &&
              *body.front().get_nonterminal_ptr() == head) {
            body.erase(body.begin());
            body.emplace_back(new_head);
            new_bodies.emplace_back(std::move(body));
            body.clear();
          }
        }

        if (new_bodies.empty()) {
          return;
        }
        new_bodies.emplace_back(1, symbol_type(alphabet->get_epsilon()));
        productions[new_head] = new_bodies;

        for (auto &body : bodies) {
          if (!body.empty()) {
            body.emplace_back(new_head);
          }
        }
      };

  for (size_t i = 0; i < old_heads.size(); i++) {
    for (size_t j = 0; j < i; j++) {
      std::vector<production_body_type> new_bodies;
      for (auto &body : productions[old_heads[i]]) {

        if (body.empty()) {
          continue;
        }

        if (!(body.front().is_nonterminal() &&
              *body.front().get_nonterminal_ptr() == old_heads[j])) {
          new_bodies.emplace_back(std::move(body));
          continue;
        }
        // Ai -> Aj，替换
        for (const auto &head_j_body : productions[old_heads[j]]) {
          auto new_body = head_j_body;
          new_body.insert(new_body.end(), body.begin() + 1, body.end());
          new_bodies.push_back(new_body);
        }
      }
      productions[old_heads[i]] = std::move(new_bodies);
    }
    eliminate_immediate_left_recursion(old_heads[i]);
  }
  normalize_productions();
}

void CFG::left_factoring() {

  normalize_productions();
  const auto left_factoring_nonterminal =
      [this](const nonterminal_type &head) -> nonterminal_type {
    auto &bodies = productions[head];
    if (bodies.size() < 2) {
      return {};
    }
    auto common_prefix = bodies.front();
    bool is_common_prefix = false;
    std::vector<size_t> indexes{0};
    for (size_t j = 1; j < bodies.size(); j++) {
      size_t i = 0;
      for (; i < common_prefix.size() && i < bodies[j].size(); i++) {
        if (common_prefix[i] != bodies[j][i]) {
          break;
        }
      }
      if (i > 0) {
        is_common_prefix = true;
        common_prefix.resize(i);
        indexes.push_back(j);
        continue;
      }

      if (is_common_prefix) {
        break;
      }
      common_prefix = bodies[j];
      indexes = {j};
    }
    if (is_common_prefix) {
      auto new_head = get_new_head(head);
      for (auto &index : indexes) {
        auto &body = bodies[index];

        productions[new_head].emplace_back(
            std::move_iterator(body.begin() + common_prefix.size()),
            std::move_iterator(body.end()));
        if (productions[new_head].back().empty()) {
          productions[new_head].back().emplace_back(alphabet->get_epsilon());
        }

        body.erase(body.begin() + common_prefix.size(), body.end());

        body.emplace_back(new_head);
      }

      return new_head;
    }
    return {};
  };

  for (auto head : get_heads()) {
    while (true) {
      head = left_factoring_nonterminal(head);
      if (head.empty()) {
        break;
      }
    }
  }

  normalize_productions();
}

bool CFG::recursive_descent_parse(symbol_string_view view) const {

  auto match_nonterminal = [&](auto &&self, const nonterminal_type &nonterminal,
                               bool check_endmark, size_t &pos,
                               const CFG &cfg) -> bool {
    if (pos >= view.size()) {
      return false;
    }

    auto it = cfg.productions.find(nonterminal);
    for (auto const &body : it->second) {
      auto local_pos = pos;
      bool match = true;
      for (size_t i = 0; i < body.size(); i++) {

        auto const &grammal_symbol = body[i];

        if (grammal_symbol.is_terminal()) {
          const auto terminal = *(grammal_symbol.get_terminal_ptr());
          if (terminal == view[local_pos]) {
            local_pos++;
          } else {
            match = false;
            break;
          }
        } else {
          auto const &this_nonterminal =
              *(grammal_symbol.get_nonterminal_ptr());
          if (self(self, this_nonterminal,
                   check_endmark && (i == body.size() - 1), local_pos, cfg)) {
            continue;
          }
          match = false;
          break;
        }
      }

      if (match) {
        if (check_endmark && local_pos < view.size()) {
          match = false;
        }
      }

      if (match) {
        pos = local_pos;
        return true;
      }
    }

    return false;
  };

  size_t start_pos = 0;
  return match_nonterminal(match_nonterminal, start_symbol, true, start_pos,
                           *this);
}

const std::map<CFG::nonterminal_type, std::set<CFG::terminal_type>> &
CFG::first() const {

  if (!first_sets.empty()) {
    return first_sets;
  }
  std::unordered_map<CFG::nonterminal_type, bool> flags;
  std::unordered_map<CFG::nonterminal_type, bool> epsilon_flags;

  // process all terminals
  for (auto const &[head, bodies] : productions) {
    for (auto const &body : bodies) {
      auto terminal_ptr = body[0].get_terminal_ptr();
      if (terminal_ptr) {
        first_sets[head].insert(*terminal_ptr);
      }
    }
    flags[head] = true;
  }

  bool add_new_terminal = true;
  while (add_new_terminal) {

    add_new_terminal = false;

    for (auto const &[head, bodies] : productions) {
      for (auto const &body : bodies) {
        size_t i = 0;
        for (; i < body.size(); i++) {
          if (body[i].is_terminal()) {
            if (first_sets[head].insert(*body[i].get_terminal_ptr()).second) {
              add_new_terminal = true;
            }
            break;
          }

          auto const &nonterminal = *(body[i].get_nonterminal_ptr());

          bool has_epsilon = false;
          for (auto const &first_terminal : first_sets[nonterminal]) {
            if (first_sets[head].insert(first_terminal).second) {
              add_new_terminal = true;
            }
            if (alphabet->is_epsilon(first_terminal)) {
              has_epsilon = true;
            }
          }
          if (!has_epsilon) {
            break;
          }
        }
        if (i == body.size()) {
          if (first_sets[head].insert(alphabet->get_epsilon()).second) {
            add_new_terminal = true;
          }
        }
      }
    }
  }

  return first_sets;
}

std::set<CFG::terminal_type>
CFG::first(const grammar_symbol_const_span_type &alpha) const {

  first();
  std::set<terminal_type> view_first_set;
  for (auto const &symbol : alpha) {

    if (symbol.is_terminal()) {
      return {*symbol.get_terminal_ptr()};
    }
    const auto &nonterminal = *symbol.get_nonterminal_ptr();

    bool has_epsilon = false;
    auto it = first_sets.find(nonterminal);
    for (auto const &terminal : it->second) {
      if (alphabet->is_epsilon(terminal)) {
        has_epsilon = true;
      } else {
        view_first_set.insert(terminal);
      }
    }
    if (!has_epsilon) {
      return view_first_set;
    }
  }
  view_first_set.insert(alphabet->get_epsilon());

  return view_first_set;
}

std::map<CFG::nonterminal_type, std::set<CFG::terminal_type>>
CFG::follow() const {

  std::map<nonterminal_type, std::set<terminal_type>> follow_sets;

  follow_sets[start_symbol].insert(alphabet->get_endmarker());

  bool has_add = false;
  do {
    has_add = false;
    for (const auto &[head, bodies] : productions) {
      for (const auto &body : bodies) {
        for (size_t i = 0; i < body.size(); i++) {
          if (body[i].is_terminal()) {
            continue;
          }

          const auto &nonterminal = *(body[i].get_nonterminal_ptr());

          auto first_set =
              first(grammar_symbol_const_span_type(body).subspan(i + 1));

          bool has_epsilon = false;

          auto &follow_set = follow_sets[nonterminal];
          for (auto const &terminal : first_set) {
            if (alphabet->is_epsilon(terminal)) {
              has_epsilon = true;
            } else {
              if (follow_set.insert(terminal).second) {
                has_add = true;
              }
            }
          }

          if (has_epsilon) {
            for (auto const &terminal : follow_sets[head]) {
              if (follow_set.insert(terminal).second) {
                has_add = true;
              }
            }
          }
        }
      }
    }
  } while (has_add);

  return follow_sets;
}

} // namespace cyy::computation
