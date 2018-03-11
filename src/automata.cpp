/*!
 * \file automata.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include <vector>

#include "automata.hpp"

namespace cyy::lang {

DFA NFA_to_DFA(const NFA &nfa) {
  std::vector<std::set<uint64_t>> subsets{
      nfa.epsilon_closure({nfa.get_start_state()})};
  std::vector<bool> flags{false};
  std::set<uint64_t> DFA_states;
  std::map<std::pair<uint64_t, symbol_type>, uint64_t> DFA_transition_table;
  std::set<uint64_t> DFA_final_states;
  for (size_t i = 0; i < flags.size(); i++) {
    if (flags[i]) {
      continue;
    }
    flags[i] = true;
    nfa.get_alphabet().foreach_symbol([&](auto const &a) {
      auto res = nfa.move(subsets[i], a);

      size_t j = 0;
      for (j = 0; j < flags.size(); j++) {
        if (subsets[j] == res) {
          break;
        }
      }

      // new state
      if (j == flags.size()) {
        subsets.push_back(res);
        flags.push_back(false);
        DFA_states.insert(j);

        if (nfa.contain_final_state(res)) {
          DFA_final_states.insert(j);
        }
      }

      DFA_transition_table[{i, a}] = j;
    });
  }

  return {DFA_states, nfa.get_alphabet().name(), 0, DFA_transition_table,
          DFA_final_states};
}

DFA DFA::minimize() const {
  std::set<uint64_t> non_final_states;
  std::set_difference(states.begin(), states.end(), final_states.begin(),
                      final_states.end(),
                      std::inserter(non_final_states, non_final_states.end()));

  std::vector<std::set<uint64_t>> groups{non_final_states, final_states};
  std::map<uint64_t, size_t> state_location;

  while (true) {
    for (size_t i = 0; i < groups.size(); i++) {
      for (auto state : groups[i]) {
        state_location[state] = i;
      }
    }

    decltype(groups) new_groups;
    for (auto const &group : groups) {
      if (group.empty()) {
        continue;
      }
      if (group.size() <= 1) {
        new_groups.push_back(group);
        continue;
      }

      decltype(groups) sub_groups;

      auto it = group.begin();
      sub_groups.push_back({*it});
      while (it != group.end()) {
        auto state = *it;
        bool in_new_group = true;
        for (auto &sub_group : sub_groups) {

          bool in_group = true;
          alphabet->foreach_symbol([&](auto const &a) {
            if (in_group && state_location[move(*(sub_group.begin()), a)] !=
                                state_location[move(state, a)]) {
              in_group = false;
            }
          });
          if (in_group) {
            sub_group.insert(state);
            in_new_group = false;
            break;
          }
        }
        if (in_new_group) {
          sub_groups.push_back({state});
        }
      }
      new_groups.insert(new_groups.end(), sub_groups.begin(), sub_groups.end());
    }
    if (groups == new_groups) {
      break;
    }
  }
  uint64_t minimize_DFA_start_state{};
  std::set<uint64_t> minimize_DFA_states;
  std::set<uint64_t> minimize_DFA_final_states;
  std::map<std::pair<uint64_t, symbol_type>, uint64_t>
      minimize_DFA_transition_table;
  for (size_t i = 0; i < groups.size(); i++) {
    minimize_DFA_states.insert(i);
    if (groups[i].count(this->start_state) != 0) {
      minimize_DFA_start_state = i;
    }

    for (auto const &state : groups[i]) {
      if (final_states.count(state) != 0) {
        minimize_DFA_final_states.insert(i);
        break;
      }
    }

    alphabet->foreach_symbol([&](auto const &a) {
      auto next_state = move(*(groups[i].begin()), a);

      minimize_DFA_transition_table[{i, a}] = state_location[next_state];
    });
  }
  return {minimize_DFA_states, alphabet->name(), minimize_DFA_start_state,
          minimize_DFA_transition_table, minimize_DFA_final_states};
}
} // namespace cyy::lang
