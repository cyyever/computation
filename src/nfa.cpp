/*!
 * \file automaton.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include <vector>

#include "automaton.hpp"

namespace cyy::lang {
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
          std::cout << "u=" << u << std::endl;
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
DFA NFA::to_DFA() const {
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

        std::cout << "new_state=" << j << std::endl;

        if (contain_final_state(res)) {
          DFA_final_states.insert(j);
        }
      }

      DFA_transition_table[{i, a}] = j;
    });
  }

  return {DFA_states, alphabet->name(), 0, DFA_transition_table,
          DFA_final_states};
}
} // namespace cyy::lang
