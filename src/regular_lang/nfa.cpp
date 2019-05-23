/*!
 * \file nfa.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include <cassert>
#include <iterator>
#include <range/v3/algorithm.hpp>
#include <vector>

#include "../util.hpp"
#include "nfa.hpp"

namespace cyy::computation {

  std::set<NFA::state_type> NFA::move(const std::set<state_type> &T,
                                      symbol_type a) const {
    std::set<state_type> direct_reachable;

    for (const auto &s : T) {
      auto it = transition_function.find({a, s});
      if (it != transition_function.end()) {
        direct_reachable.insert(it->second.begin(), it->second.end());
      }
    }

    std::set<state_type> res;
    for (auto const &d : direct_reachable) {
      auto const &closure = epsilon_closure(d);
      res.insert(closure.begin(), closure.end());
    }
    return res;
  }
  bool NFA::simulate(symbol_string_view view) const {
    auto s = epsilon_closure(start_state);
    for (auto const &symbol : view) {
      s = move(s, symbol);
      if (s.empty()) {
        return false;
      }
    }
    return contain_final_state(s);
  }

  const std::set<NFA::state_type> &NFA::epsilon_closure(state_type s) const {
    auto it = epsilon_closures.find(s);
    if (it != epsilon_closures.end()) {
      return it->second;
    }
    std::map<state_type, std::set<state_type>> dependency;
    std::set<state_type> unstable_states{s};
    std::vector<state_type> stack{s};
    for (size_t i = 0; i < stack.size(); i++) {
      auto unstable_state = stack[i];
      auto it2 = epsilon_transition_function.find(unstable_state);
      if (it2 == epsilon_transition_function.end()) {
        continue;
      }

      for (auto next_state : it2->second) {
        auto it3 = epsilon_closures.find(next_state);
        if (it3 != epsilon_closures.end()) {
          epsilon_closures[unstable_state].insert(it3->second.begin(),
                                                  it3->second.end());
        } else {
          if (unstable_states.insert(next_state).second) {
            stack.push_back(next_state);
          }
          dependency[next_state].insert(unstable_state);
        }
      }
    }

    for (auto unstable_state : unstable_states) {
      epsilon_closures[unstable_state].insert(unstable_state);
    }

    auto [sorted_states, remain_dependency] = topological_sort(dependency);

    for (auto sorted_state : sorted_states) {
      for (auto prev_state : dependency[sorted_state]) {
        std::set<state_type> diff;
        auto &prev_epsilon_closure = epsilon_closures[prev_state];
        auto &unstable_epsilon_closure = epsilon_closures[sorted_state];
        ranges::v3::set_difference(unstable_epsilon_closure,
                                   prev_epsilon_closure,
                                   ranges::v3::inserter(diff, diff.begin()));

        if (!diff.empty()) {
          prev_epsilon_closure.merge(std::move(diff));
        }
      }
      unstable_states.erase(sorted_state);
    }

    while (!unstable_states.empty()) {
      auto it2 = unstable_states.begin();
      auto unstable_state = *it2;
      unstable_states.erase(it2);
      for (auto prev_state : remain_dependency[unstable_state]) {
        std::set<state_type> diff;
        auto &prev_epsilon_closure = epsilon_closures[prev_state];
        auto &unstable_epsilon_closure = epsilon_closures[unstable_state];
        ranges::v3::set_difference(unstable_epsilon_closure,
                                   prev_epsilon_closure,
                                   ranges::v3::inserter(diff, diff.begin()));

        if (!diff.empty()) {
          prev_epsilon_closure.merge(std::move(diff));
          unstable_states.insert(prev_state);
        }
      }
    }
    return epsilon_closures[s];
  }

  std::pair<DFA, std::unordered_map<DFA::state_type, std::set<NFA::state_type>>>
  NFA::to_DFA_with_mapping() const {
    std::map<std::set<state_type>, state_type> subsets{
        {epsilon_closure(start_state), 0}};
    state_type next_state = 1;

    std::optional<state_type> empty_set_state;
    DFA::transition_function_type DFA_transition_function;

    std::set<symbol_type> active_symbols;
    for (auto const &[p, _] : transition_function) {
      active_symbols.insert(p.first);
    }
    std::set<symbol_type> inactive_symbols;
    alphabet->foreach_symbol(
        [&active_symbols, &inactive_symbols](auto const &a) {
          if (active_symbols.count(a) == 0) {
            inactive_symbols.insert(a);
          }
        });

    for (auto const &[subset, state] : subsets) {
      for (auto a : inactive_symbols) {
        if (!empty_set_state) {
          auto [_, has_emplaced] = subsets.try_emplace({}, next_state);
          assert(has_emplaced);
          empty_set_state = next_state;
          next_state++;
        }
        DFA_transition_function[{a, empty_set_state.value()}] =
            empty_set_state.value();
      }

      for (auto a : active_symbols) {
        auto res = move(subset, a);
        auto [it2, has_emplaced] =
            subsets.try_emplace(std::move(res), next_state);
        if (has_emplaced) {
          next_state++;
        }
        DFA_transition_function[{a, state}] = it2->second;
      }
    }

    std::set<state_type> DFA_states;
    std::set<state_type> DFA_final_states;
    std::unordered_map<state_type, std::set<state_type>> state_map;
    for (auto const &[subset, DFA_state] : subsets) {
      DFA_states.insert(DFA_state);
      if (contain_final_state(subset)) {
        DFA_final_states.insert(DFA_state);
      }
      state_map.emplace(DFA_state, subset);
    }

    return {{DFA_states, alphabet->get_name(), 0, DFA_transition_function,
             DFA_final_states},
            state_map};
  }

  DFA NFA::to_DFA() const { return to_DFA_with_mapping().first; }
} // namespace cyy::computation
