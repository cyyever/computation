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

  std::set<uint64_t> NFA::move(const std::set<uint64_t> &T,
                               symbol_type a) const {
    std::set<uint64_t> direct_reachable;

    for (const auto &s : T) {
      auto it = transition_table.find({s, a});
      if (it != transition_table.end()) {
        direct_reachable.insert(it->second.begin(), it->second.end());
      }
    }

    return epsilon_closure(direct_reachable);
  }

  std::set<uint64_t> NFA::epsilon_closure(const std::set<uint64_t> &T) const {

    auto stack = T;
    auto res = T;
    auto epsilon = alphabet->get_epsilon();

    while (!stack.empty()) {
      decltype(stack) next_stack;
      for (auto const &t : stack) {
        auto it = transition_table.find({t, epsilon});
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
    return res;
  }

  std::pair<DFA, std::unordered_map<uint64_t, std::set<uint64_t>>>
  NFA::to_DFA_with_mapping() const {
    std::map<std::set<uint64_t>, uint64_t> subsets{
        {epsilon_closure({start_state}), 0}};
    uint64_t next_state = 1;

    std::map<std::pair<uint64_t, symbol_type>, uint64_t> DFA_transition_table;
    for (auto it = subsets.begin(); it != subsets.end(); it++) {
      alphabet->foreach_symbol([&](auto const &a) {
        auto res = move(it->first, a);

        auto [it2, has_emplaced] = subsets.emplace(std::move(res), next_state);
        if (has_emplaced) {
          next_state++;
        }
        DFA_transition_table[{it->second, a}] = it2->second;
      });
    }

    std::set<uint64_t> DFA_states;
    std::set<uint64_t> DFA_final_states;
    std::unordered_map<uint64_t, std::set<uint64_t>> state_map;
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
