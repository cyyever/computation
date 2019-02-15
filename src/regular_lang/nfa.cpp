/*!
 * \file nfa.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include <cassert>
#include <vector>

#include "nfa.hpp"

namespace cyy::computation {

  std::set<NFA::state_type> NFA::move(const std::set<state_type> &T,
                                      symbol_type a) const {
    std::set<state_type> direct_reachable;

    for (const auto &s : T) {
      auto it = transition_table.find({a, s});
      if (it != transition_table.end()) {
        direct_reachable.insert(it->second.begin(), it->second.end());
      }
    }

    /*
    std::set<state_type> res;
    for(auto const & d:direct_reachable) {
      res.merge(std::set<state_type>(epsilon_closure(d)));
    }
    return res;
    */


    return epsilon_closure(direct_reachable);
  }

  const std::set<NFA::state_type> &NFA::epsilon_closure(state_type s) const {
    if (!epsilon_closures.empty()) {
      return epsilon_closures[s];
    }

    for (auto a : states) {
      epsilon_closures[a].insert(a);
    };

    std::map<state_type, std::vector<state_type>> dependency;
    std::set<state_type> unstable_states;
    auto epsilon = alphabet->get_epsilon();
    for (auto const &[p, next_states] : transition_table) {
      if (p.first != epsilon) {
        continue;
      }
      epsilon_closures[p.second].merge(std::set<state_type>(next_states));
      for (auto next_state : next_states) {
        dependency[next_state].push_back(p.second);
      }
      unstable_states.insert(p.second);
    }

    while (!unstable_states.empty()) {
      auto it = unstable_states.begin();
      auto unstable_state = *it;
      unstable_states.erase(it);
      for (auto prev_state : dependency[unstable_state]) {
        std::set<state_type> diff;
        auto &prev_epsilon_closure = epsilon_closures[prev_state];
        auto &unstable_epsilon_closure = epsilon_closures[unstable_state];
        std::set_difference(
            unstable_epsilon_closure.begin(), unstable_epsilon_closure.end(),
            prev_epsilon_closure.begin(), prev_epsilon_closure.end(),
            std::inserter(diff, diff.begin()));

        if (!diff.empty()) {
          prev_epsilon_closure.merge(diff);
          unstable_states.insert(prev_state);
        }
      }
    }
    return epsilon_closures[s];
  }

  std::set<NFA::state_type>
  NFA::epsilon_closure(const std::set<state_type> &T) const {

    auto stack = T;
    auto res = T;
    auto epsilon = alphabet->get_epsilon();

    while (!stack.empty()) {
      decltype(stack) next_stack;
      for (auto const &t : stack) {
        auto it = transition_table.find({epsilon, t});
        if (it == transition_table.end()) {
          continue;
        }
        for (auto const &u : it->second) {
          if (res.count(u) == 0) {
            next_stack.insert(u);
            res.insert(u);
          }
        }
      }
      stack = std::move(next_stack);
    }

    if (T == std::set<state_type>{0}) {
      for (auto a : res) {
        std::cout << "epsilon_closures next_stack is " << (int)a << std::endl;
      }
    }
    return res;
  }

  std::pair<DFA, std::unordered_map<DFA::state_type, std::set<NFA::state_type>>>
  NFA::to_DFA_with_mapping() const {
    std::map<std::set<state_type>, state_type> subsets{
        {epsilon_closure(start_state), 0}};
    state_type next_state = 1;

    DFA::transition_table_type DFA_transition_table;
    for (auto it = subsets.begin(); it != subsets.end(); it++) {
      alphabet->foreach_symbol([&](auto const &a) {
        auto res = move(it->first, a);

        auto [it2, has_emplaced] =
            subsets.try_emplace(std::move(res), next_state);
        if (has_emplaced) {
          next_state++;
        }
        DFA_transition_table[{a, it->second}] = it2->second;
      });
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

    return {{DFA_states, alphabet->get_name(), 0, DFA_transition_table,
             DFA_final_states},
            state_map};
  }

  DFA NFA::to_DFA() const { return to_DFA_with_mapping().first; }
} // namespace cyy::computation
