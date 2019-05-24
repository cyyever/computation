/*!
 * \file nfa.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include "gnfa.hpp"

namespace cyy::computation {
  GNFA::GNFA(const DFA &dfa) : finite_automaton(dfa) {
    for (auto const &[p, next_state] : dfa.get_transition_function()) {
      auto const &[symbol, state] = p;
      auto node = std::make_shared<regex::basic_node>(symbol);
      auto &regex_syntax_node = transition_function[{state, next_state}];
      if (!regex_syntax_node) {
        regex_syntax_node = std::move(node);
      } else {
        regex_syntax_node =
            std::make_shared<regex::union_node>(regex_syntax_node, node);
      }
    }
    auto new_start_state = add_new_state();
    transition_function[{new_start_state, start_state}] =
        std::make_shared<regex::epsilon_node>();
    change_start_state(new_start_state);
    auto new_final_state = add_new_state();
    for (auto final_state : get_final_states()) {
      transition_function[{final_state, new_final_state}] =
          std::make_shared<regex::epsilon_node>();
    }
    change_final_states({new_final_state});
  }

} // namespace cyy::computation
