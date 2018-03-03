/*!
 * \file automata.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include <vector>

#include "automata.hpp"

namespace cyy::compiler {

DFA NFA_to_DFA(const NFA &nfa) {
  using state_type = NFA::state_type;
  using symbol_type = NFA::symbol_type;
  std::vector<std::set<state_type>> subsets{
      nfa.epsilon_closure({nfa.get_start_state()})};
  std::vector<bool> flags{false};
  std::set<state_type> DFA_states;
  std::map<std::pair<state_type, symbol_type>, state_type> DFA_transition_table;
  std::set<state_type> DFA_final_states;
  for (size_t i = 0; i < flags.size(); i++) {
    if (flags[i]) {
      continue;
    }
    flags[i] = true;
    for (auto const &a : nfa.get_alphabet()) {

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
    }
  }

  return {DFA_states, nfa.get_alphabet(), 0, DFA_transition_table,
          DFA_final_states};
}

} // namespace cyy::compiler
