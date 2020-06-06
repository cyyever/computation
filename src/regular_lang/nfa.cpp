/*!
 * \file nfa.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include <algorithm>
#include <cassert>
#include <iterator>
#include <vector>

#include "../util.hpp"
#include "nfa.hpp"

namespace cyy::computation {

  std::set<NFA::state_type> NFA::go(const state_set_type &T,
                                    symbol_type a) const {
    state_set_type direct_reachable;

    for (const auto &s : T) {
      auto it = transition_function.find({s, a});
      if (it != transition_function.end()) {
        direct_reachable.insert(it->second.begin(), it->second.end());
      }
    }

    state_set_type res;
    for (auto const &d : direct_reachable) {
      auto const &closure = get_epsilon_closure(d, epsilon_transition_function);
      res.insert(closure.begin(), closure.end());
    }
    return res;
  }
  bool NFA::recognize(symbol_string_view view) const {
    auto s = get_epsilon_closure(start_state, epsilon_transition_function);
    for (auto const &symbol : view) {
      s = go(s, symbol);
      if (s.empty()) {
        return false;
      }
    }
    return contain_final_state(s);
  }

  std::pair<DFA, std::unordered_map<DFA::state_type, std::set<NFA::state_type>>>
  NFA::to_DFA_with_mapping() const {
    state_type next_state = 1;
    DFA::transition_function_type DFA_transition_function;
    std::map<state_set_type, state_type> subsets{
        {get_epsilon_closure(start_state, epsilon_transition_function), 0}};
    std::vector<decltype(subsets.begin())> tmp_states{subsets.begin()};
    for (size_t i = 0; i < tmp_states.size(); i++) {
      for (auto a : *alphabet) {
        auto const &[subset, state] = *tmp_states[i];
        auto res = go(subset, a);
        auto [it, has_emplaced] = subsets.emplace(std::move(res), next_state);
        if (has_emplaced) {
          tmp_states.emplace_back(it);
          next_state++;
        }
        DFA_transition_function[{state, a}] = it->second;
      }
    }

    state_set_type DFA_states;
    state_set_type DFA_final_states;
    std::unordered_map<state_type, state_set_type> state_map;
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
