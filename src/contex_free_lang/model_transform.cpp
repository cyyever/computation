/*!
 * \file model_transform.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "model_transform.hpp"

namespace cyy::computation {

CFG NFA_to_CFG(const NFA &nfa) {
  std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
      productions;

  auto state_to_nonterminal = [](symbol_type state) {
    return std::string("S") + std::to_string(state);
  };

  for (auto const &[p, next_states] : nfa.get_transition_table()) {
    auto const &[cur_state, symbol] = p;
    for (auto const &next_state : next_states) {
      if (symbol != nfa.get_alphabet().get_epsilon()) {
        productions[state_to_nonterminal(cur_state)].push_back(
            CFG::production_body_type{{symbol},
                                      {state_to_nonterminal(next_state)}});
      } else {
        productions[state_to_nonterminal(cur_state)].push_back(
            CFG::production_body_type{{state_to_nonterminal(next_state)}});
      }
    }
  }

  for (auto const &final_state : nfa.get_final_states()) {
    productions[state_to_nonterminal(final_state)].push_back(
        CFG::production_body_type{{nfa.get_alphabet().get_epsilon()}});
  }

  return {nfa.get_alphabet().name(),
          state_to_nonterminal(nfa.get_start_state()), productions};
}

} // namespace cyy::computation
