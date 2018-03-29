/*!
 * \file grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "grammar.hpp"
#include <stack>

namespace cyy::lang {

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
        } else if (it->second == nonterminal_state::checking) {
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
        auto nonterminal = std::get<nonterminal_type>(symbol);
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

void CFG::eliminate_left_recursion() {

  std::vector<nonterminal_type> old_heads{start_symbol};

  for (const auto &[head, _] : productions) {
    if (head != start_symbol) {
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
            body.push_back(new_head);
          }
        }
      };

  for (size_t i = 0; i < old_heads.size(); i++) {
    for (size_t j = 0; j < i; j++) {
      std::vector<production_body_type> new_bodies;
      for (auto &body : productions[old_heads[i]]) {

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
  return;
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

        body.push_back(new_head);
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
          } else {
            match = false;
            break;
          }
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

std::map<CFG::nonterminal_type, std::set<CFG::terminal_type>>
CFG::first() const {

  std::map<CFG::nonterminal_type, std::set<CFG::terminal_type>> first_sets;
  auto first_of_nonterminal = [&](auto &&self, const nonterminal_type &head,
                                  const CFG &cfg) -> bool {
    bool add_new_terminal = false;
    while (true) {
      bool loop_add_new_terminal = false;
      auto it = cfg.productions.find(head);
      if (it == cfg.productions.end()) {
        continue;
      }
      for (auto const &body : it->second) {
        size_t i = 0;
        for (; i < body.size(); i++) {
          if (std::holds_alternative<terminal_type>(body[i])) {
            if (first_sets[head]
                    .insert(std::get<terminal_type>(body[i]))
                    .second) {
              loop_add_new_terminal = true;
            }
            break;
          }
          auto nonterminal = std::get<nonterminal_type>(body[i]);
          if (self(self, nonterminal, cfg)) {
            loop_add_new_terminal = true;
          }

          bool has_epsilon = false;
          for (auto const &terminal : first_sets[nonterminal]) {
            if (alphabet->is_epsilon(terminal)) {
              has_epsilon = true;
              continue;
            }
            if (first_sets[head].insert(terminal).second) {
              loop_add_new_terminal = true;
            }
          }
          if (!has_epsilon) {
            break;
          }
        }
        if (i == body.size()) {
          if (first_sets[head].insert(alphabet->get_epsilon()).second)
            loop_add_new_terminal = true;
        }
      }
      if (!loop_add_new_terminal) {
        break;
      }
    }

    return add_new_terminal;
  };

  for (auto const &[head, _] : productions) {
    first_of_nonterminal(first_of_nonterminal, head, *this);
  }
  return first_sets;
}

std::set<CFG::terminal_type>
CFG::first(const grammar_symbol_string_view &alpha,
           const std::map<nonterminal_type, std::set<terminal_type>>
               &nonterminal_first_sets) const {
  std::set<terminal_type> first_set;
  for (auto const &symbol : alpha) {

    if (std::holds_alternative<terminal_type>(symbol)) {
      return {std::get<terminal_type>(symbol)};
    }
    auto nonterminal = std::get<nonterminal_type>(symbol);

    bool has_epsilon = false;
    ;
    auto it = nonterminal_first_sets.find(nonterminal);
    for (auto const &terminal : it->second) {
      if (alphabet->is_epsilon(terminal)) {
        has_epsilon = true;
      } else {
        first_set.insert(terminal);
      }
    }
    if (!has_epsilon) {
      return first_set;
    }
  }
  first_set.insert(alphabet->get_epsilon());

  return first_set;
}

std::map<CFG::nonterminal_type, std::set<CFG::terminal_type>>
CFG::follow(const std::map<nonterminal_type, std::set<terminal_type>>
                &nonterminal_first_sets) const {

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

          auto nonterminal = std::get<nonterminal_type>(body[i]);

          auto first_set =
              first({body.data() + i, body.size() - i}, nonterminal_first_sets);

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

bool CFG::is_LL1(const std::map<nonterminal_type, std::set<terminal_type>>
                     &nonterminal_first_sets,
                 const std::map<nonterminal_type, std::set<terminal_type>>
                     &follow_sets) const {

  auto has_intersection = [](const auto &set1, const auto &set2) {
    auto it1 = set1.begin();
    auto it2 = set2.begin();
    while (it1 != set1.end() && it2 != set2.end()) {
      if (*it1 < *it2) {
        it1++;
      } else if (*it1 == *it2) {
        return true;
      } else {
        it2++;
      }
    }
    return false;
  };

  for (const auto &[head, bodies] : productions) {

    std::vector<std::set<CFG::terminal_type>> first_sets;
    auto follow_it = follow_sets.find(head);
    for (size_t i = 0; i < bodies.size(); i++) {
      first_sets.emplace_back(
          first({bodies[i].data(), bodies[i].size()}, nonterminal_first_sets));
      for (size_t j = 0; j < i; j++) {
        if (has_intersection(first_sets[i], first_sets[j])) {
          return false;
        }

        if (first_sets[i].count(alphabet->get_epsilon()) != 0 &&
            has_intersection(follow_it->second, first_sets[j])) {
          return false;
        }

        if (first_sets[j].count(alphabet->get_epsilon()) != 0 &&
            has_intersection(follow_it->second, first_sets[i])) {
          return false;
        }
      }
    }
  }
  return true;
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
} // namespace cyy::lang
