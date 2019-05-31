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

  void GNFA::remove_state(state_type removed_state) {
    for (auto from_state : states) {
      if (from_state == removed_state || is_final_state(removed_state)) {
        continue;
      }
      for (auto to_state : states) {
        if (to_state == removed_state || to_state == start_state) {
          continue;
        }
        auto &from_to_regex_expr = transition_function[{from_state, to_state}];
        if (!from_to_regex_expr) {
          from_to_regex_expr = std::make_shared<regex::empty_set_node>();
        }
        auto &from_removed_regex_expr =
            transition_function[{from_state, removed_state}];
        if (!from_removed_regex_expr) {
          from_removed_regex_expr = std::make_shared<regex::empty_set_node>();
        }

        auto &removed_removed_regex_expr =
            transition_function[{removed_state, to_state}];
        if (!removed_removed_regex_expr) {
          removed_removed_regex_expr =
              std::make_shared<regex::empty_set_node>();
        }
        auto &removed_to_regex_expr =
            transition_function[{removed_state, to_state}];
        if (!removed_to_regex_expr) {
          removed_to_regex_expr = std::make_shared<regex::empty_set_node>();
        }
        from_to_regex_expr =
            std::make_shared<regex::union_node>(
                from_to_regex_expr,
                std::make_shared<regex::concat_node>(
                    from_removed_regex_expr,
                    std::make_shared<regex::concat_node>(
                        std::make_shared<regex::kleene_closure_node>(
                            removed_removed_regex_expr),
                        removed_to_regex_expr))

                    )
                ->simplify();
      }
    }
    states.erase(removed_state);
  }
} // namespace cyy::computation
