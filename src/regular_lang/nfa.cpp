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
    std::vector<std::set<uint64_t>> subsets{epsilon_closure({start_state})};

    std::vector<bool> flags{false};
    std::set<uint64_t> DFA_states{0};
    std::map<std::pair<uint64_t, symbol_type>, uint64_t> DFA_transition_table;
    std::set<uint64_t> DFA_final_states;
    for (size_t i = 0; i < flags.size(); i++) {
      if (flags[i]) {
        continue;
      }
      flags[i] = true;
      alphabet->foreach_symbol([&](auto const &a) {
        auto res = move(subsets[i], a);

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
        }

        DFA_transition_table[{i, a}] = j;
      });
    }

    for (size_t i = 0; i < subsets.size(); i++) {
      if (contain_final_state(subsets[i])) {
        DFA_final_states.insert(i);
      }
    }
    assert(subsets.size() == DFA_states.size());

    std::unordered_map<uint64_t, std::set<uint64_t>> state_map;
    for (size_t DFA_state = 0; DFA_state < subsets.size(); DFA_state++) {
      state_map.emplace(DFA_state, std::move(subsets[DFA_state]));
    }

    return {{DFA_states, alphabet->get_name(), 0, DFA_transition_table,
             DFA_final_states},
            state_map};
  }

  DFA NFA::to_DFA() const { return to_DFA_with_mapping().first; }
} // namespace cyy::computation
