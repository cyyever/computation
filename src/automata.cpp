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

} // namespace cyy::lang
