/*!
 * \file dk_base.cpp
 *
 */

#include "dk_base.hpp"

namespace cyy::computation {
  DK_DFA_base::DK_DFA_base(const CFG &cfg)
      : alphabet_of_nonterminals(cfg.get_nonterminal_alphabet()) {}

  DK_DFA_base::goto_table_type
  DK_DFA_base::get_goto_table(bool skip_fail_state) const {
    goto_table_type goto_table;
    for (auto const &[situation, next_state] :
         dfa_ptr->get_transition_function()) {
      if (skip_fail_state) {
        if (!dfa_ptr->is_live_state(situation.state)) {
          continue;
        }
        if (!dfa_ptr->is_live_state(next_state)) {
          continue;
        }
      }
      if (alphabet_of_nonterminals->contain(situation.input_symbol)) {
        goto_table[{situation.state, alphabet_of_nonterminals->get_data(
                                         situation.input_symbol)}] = next_state;
      } else {
        goto_table[{situation.state, situation.input_symbol}] = next_state;
      }
    }
    return goto_table;
  }

} // namespace cyy::computation
