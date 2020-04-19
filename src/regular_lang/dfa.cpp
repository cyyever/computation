/*!
 * \file dfa.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include <iterator>
#include <memory>
#include <range/v3/algorithm.hpp>
#include <range/v3/iterator/insert_iterators.hpp>
#include <set>
#include <vector>

#include "dfa.hpp"

namespace cyy::computation {
  bool DFA::equivalent_with(const DFA &rhs) const {

    if (alphabet != rhs.alphabet) {
      return false;
    }
    if (states.size() != rhs.states.size()) {
      return false;
    }
    if (final_states.size() != rhs.final_states.size()) {
      return false;
    }

    if (transition_function.size() != rhs.transition_function.size()) {
      return false;
    }

    std::map<state_type, state_type> state_map{{start_state, rhs.start_state}};
    while (true) {

      bool new_mapping = false;

      for (auto const &[pair, my_next_state] : transition_function) {
        auto const &[my_next_symbol, my_state] = pair;
        auto it = state_map.find(my_state);
        if (it == state_map.end()) {
          continue;
        }

        auto it2 = rhs.transition_function.find({my_next_symbol, it->second});
        if (it2 == rhs.transition_function.end()) {
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

    return ranges::all_of(
        final_states, [&rhs](auto s) { return rhs.final_states.contains(s); });
  }

  DFA DFA::minimize() const {
    state_set_type non_final_states;
    ranges::set_difference(
        states, final_states,
        ranges::inserter(non_final_states, non_final_states.begin()));

    std::vector<state_set_type> groups{non_final_states, final_states};
    std::unordered_map<state_type, size_t> state_location;

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
            for (auto a : *alphabet) {
              if (in_group &&
                  state_location[move(*(sub_group.begin()), a).value()] !=
                      state_location[move(state, a).value()]) {
                in_group = false;
              }
            }
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
    state_type minimize_DFA_start_state{};
    state_set_type minimize_DFA_states;
    state_set_type minimize_DFA_final_states;
    transition_function_type minimize_DFA_transition_function;
    for (size_t i = 0; i < groups.size(); i++) {
      minimize_DFA_states.insert(i);
      if (groups[i].contains(this->start_state)) {
        minimize_DFA_start_state = i;
      }

      for (auto const &state : groups[i]) {
        if (final_states.contains(state)) {
          minimize_DFA_final_states.insert(i);
          break;
        }
      }

      for (auto a : *alphabet) {
        auto next_state = move(*(groups[i].begin()), a).value();

        minimize_DFA_transition_function[{a, i}] = state_location[next_state];
      }
    }
    return {minimize_DFA_states, alphabet->get_name(), minimize_DFA_start_state,
            minimize_DFA_transition_function, minimize_DFA_final_states};
  }
  void DFA::mark_live_states() const {
    if (live_states_opt.has_value()) {
      return;
    }
    std::map<state_type, state_set_type> state_dep;
    for (auto const &[p, next_state] : transition_function) {
      state_dep[next_state].insert(p.second);
    }
    auto live_states = final_states;
    while (true) {
      auto live_state_size = live_states.size();
      state_set_type new_live_states;
      for (auto s : live_states) {
        new_live_states.merge(state_set_type(state_dep[s]));
      }
      live_states.merge(std::move(new_live_states));
      if (live_states.size() == live_state_size) {
        break;
      }
    }
    live_states_opt = live_states;
    return;
  }
} // namespace cyy::computation
