/*!
 * \file dfa.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include <algorithm>
#include <iterator>
#include <set>
#include <vector>

#include "dfa.hpp"

namespace cyy::computation {
bool DFA::equivalent_with(const DFA &rhs) {

  if (alphabet != rhs.alphabet) {
    return false;
  }
  if (states.size() != rhs.states.size()) {
    std::cout << "size1 " << states.size() << " size2=" << rhs.states.size()
              << std::endl;
    return false;
  }
  if (final_states.size() != rhs.final_states.size()) {
    return false;
  }

  if (transition_table.size() != rhs.transition_table.size()) {
    return false;
  }

  std::map<uint64_t, uint64_t> state_map{{start_state, rhs.start_state}};
  std::map<uint64_t, bool> check_flags{{start_state, false}};
  while (true) {

    bool new_mapping = false;

    for (auto const &[pair, my_next_state] : transition_table) {
      auto const &[my_state, my_next_symbol] = pair;
      auto it = state_map.find(my_state);
      if (it == state_map.end()) {
        continue;
      }

      auto it2 = rhs.transition_table.find({it->second, my_next_symbol});
      if (it2 == rhs.transition_table.end()) {
        return false;
      }
      auto rhs_next_state = it2->second;

      auto [it3, has_insert] =
          state_map.insert({my_next_state, rhs_next_state});
      if (has_insert) {
        new_mapping = true;
      } else {
        if (it3->second != rhs_next_state) {
          return false;
        }
      }
    }

    if (!new_mapping) {
      break;
    }
  }
  if (state_map.size() != states.size()) {
    return false;
  }

  for (auto const &final_state : final_states) {
    if (!rhs.final_states.count(state_map[final_state])) {
      return false;
    }
  }
  return true;
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

    bool has_new_group = false;
    decltype(groups) new_groups;
    for (auto const &group : groups) {
      if (group.size() <= 1) {
        new_groups.push_back(group);
        continue;
      }

      decltype(groups) sub_groups;

      auto it = group.begin();
      sub_groups.push_back({*it});
      it++;
      for (; it != group.end(); it++) {
        auto state = *it;
        bool in_new_group = true;
        for (auto &sub_group : sub_groups) {
          bool in_group = true;
          alphabet->foreach_symbol([&](auto const &a) {
            if (in_group &&
                state_location[move(*(sub_group.begin()), a).value()] !=
                    state_location[move(state, a).value()]) {
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
          has_new_group = true;
          sub_groups.push_back({state});
        }
      }
      new_groups.insert(new_groups.end(),
                        std::move_iterator(sub_groups.begin()),

                        std::move_iterator(sub_groups.end()));
    }
    if (!has_new_group) {
      break;
    }
    groups = std::move(new_groups);
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
      auto next_state = move(*(groups[i].begin()), a).value();

      minimize_DFA_transition_table[{i, a}] = state_location[next_state];
    });
  }
  return {minimize_DFA_states, alphabet->name(), minimize_DFA_start_state,
          minimize_DFA_transition_table, minimize_DFA_final_states};
}
} // namespace cyy::computation
