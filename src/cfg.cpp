/*!
 * \file grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "cfg.hpp"
#include "automaton.hpp"

namespace cyy::computation {

CFG::CFG(
    const std::string &alphabet_name, const nonterminal_type &start_symbol_,
    std::map<nonterminal_type, std::vector<production_body_type>> &productions_)
    : alphabet(ALPHABET::get(alphabet_name)), start_symbol(start_symbol_),
      productions(productions_) {

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
        auto terminal_ptr = std::get_if<terminal_type>(&symbol);
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
        if (!std::holds_alternative<nonterminal_type>(symbol)) {
          continue;
        }
        auto nonterminal = std::get<nonterminal_type>(symbol);
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
        if (!std::holds_alternative<nonterminal_type>(symbol)) {
          continue;
        }
        const auto &nonterminal = std::get<nonterminal_type>(symbol);
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
  productions = new_productions;
}

void CFG::eliminate_left_recursion(std::vector<nonterminal_type> old_heads) {

  if (old_heads.empty()) {
    for (const auto &[head, _] : productions) {
      old_heads.push_back(head);
    }
  }

  auto eliminate_immediate_left_recursion =
      [this](const nonterminal_type &head) {
        auto new_head = get_new_head(head);
        std::vector<production_body_type> new_bodies;
        auto &bodies = productions[head];

        for (auto &body : bodies) {

          if (std::holds_alternative<nonterminal_type>(body.front()) &&
              std::get<nonterminal_type>(body.front()) == head) {
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

        if (!(std::holds_alternative<nonterminal_type>(body.front()) &&
              std::get<nonterminal_type>(body.front()) == old_heads[j])) {
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

void CFG::eliminate_epsilon_productions() {
  auto nullable_nonterminals = nullable();

  if (nullable_nonterminals.empty()) {
    return;
  }
  production_body_type epsilon_production{alphabet->get_epsilon()};

  auto heads = get_heads();
  while (!heads.empty()) {
    auto checking_heads = std::move(heads);
    heads.clear();
    for (auto const &head : checking_heads) {

      std::vector<production_body_type> new_bodies;
      for (auto &body : productions[head]) {
        if (body.empty()) {
          continue;
        }
        if (body == epsilon_production) {
          body.clear();
          continue;
        }
        for (size_t i = 0; i < body.size(); i++) {
          auto ptr = std::get_if<nonterminal_type>(&(body[i]));
          if (ptr && nullable_nonterminals.count(*ptr)) {
            nonterminal_type new_head;
            if (i + 1 < body.size()) {
              new_head = get_new_head(head);
              productions[new_head].emplace_back(
                  std::move_iterator(body.begin() + i + 1),
                  std::move_iterator(body.end()));
              body.erase(body.begin() + i + 1, body.end());
              body.push_back(new_head);
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

void CFG::left_factoring() {

  normalize_productions();
  auto left_factoring_nonterminal =
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

        if (std::holds_alternative<terminal_type>(grammal_symbol)) {
          auto terminal = std::get<terminal_type>(grammal_symbol);
          if (terminal == view[local_pos]) {
            local_pos++;
          } else {
            match = false;
            break;
          }
        } else {
          auto this_nonterminal = std::get<nonterminal_type>(grammal_symbol);
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
      auto terminal_ptr = std::get_if<terminal_type>(&body[0]);
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
          if (std::holds_alternative<terminal_type>(body[i])) {
            if (first_sets[head]
                    .insert(std::get<terminal_type>(body[i]))
                    .second) {
              add_new_terminal = true;
            }
            break;
          }

          auto const &nonterminal = std::get<nonterminal_type>(body[i]);

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
CFG::first(const grammar_symbol_string_view &alpha) const {

  first();
  std::set<terminal_type> view_first_set;
  for (auto const &symbol : alpha) {

    if (std::holds_alternative<terminal_type>(symbol)) {
      return {std::get<terminal_type>(symbol)};
    }
    const auto &nonterminal = std::get<nonterminal_type>(symbol);

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
          if (std::holds_alternative<terminal_type>(body[i])) {
            continue;
          }

          const auto &nonterminal = std::get<nonterminal_type>(body[i]);

          auto first_set = first(grammar_symbol_string_view{
              body.data() + i + 1, body.size() - i - 1});

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

CFG NFA_to_CFG(const NFA &nfa) {
  std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
      productions;

  auto state_to_nonterminal = [](symbol_type state) {
    return std::string("S") + std::to_string(state);
  };

  for (auto const &[p, next_states] : nfa.get_transition_table()) {
    auto const &[cur_state, symbol] = p;
    for (auto const &next_state : next_states) {
      if (symbol != nfa.get_alphabet().get_epsilon()) {
        productions[state_to_nonterminal(cur_state)].push_back(
            CFG::production_body_type{{symbol},
                                      {state_to_nonterminal(next_state)}});
      } else {
        productions[state_to_nonterminal(cur_state)].push_back(
            CFG::production_body_type{{state_to_nonterminal(next_state)}});
      }
    }
  }

  for (auto const &final_state : nfa.get_final_states()) {
    productions[state_to_nonterminal(final_state)].push_back(
        CFG::production_body_type{{nfa.get_alphabet().get_epsilon()}});
  }

  return {nfa.get_alphabet().name(),
          state_to_nonterminal(nfa.get_start_state()), productions};
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
          auto terminal_ptr = std::get_if<terminal_type>(&symbol);
          if (terminal_ptr) {
            if (!alphabet->is_epsilon(*terminal_ptr)) {
              body_nullable = false;
              break;
            }
            continue;
          }

          if (res.count(std::get<nonterminal_type>(symbol)) == 0) {
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
      auto ptr = std::get_if<nonterminal_type>(&body.front());
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
      std::vector<production_body_type> new_bodies;
      for (auto &body : productions[head]) {
        if (body.empty()) {
          continue;
        }
        for (size_t i = 0; i < body.size(); i++) {
          auto ptr = std::get_if<nonterminal_type>(&(body[i]));
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
            body.push_back(new_head);
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
          auto terminal_ptr = std::get_if<terminal_type>(&symbol);
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
                production_body_type{*terminal_ptr});
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
        if (!std::holds_alternative<terminal_type>(body[0])) {

          return false;
        }
      } else if (body.size() == 2) {

        if (!std::holds_alternative<nonterminal_type>(body[0])) {
          return false;
        }
        if (!std::holds_alternative<nonterminal_type>(body[1])) {
          return false;
        }
      }
    }
  }
  return true;
}

} // namespace cyy::computation
