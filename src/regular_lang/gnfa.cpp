/*!
 * \file nfa.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include "gnfa.hpp"

#include <memory>

namespace cyy::computation {
  GNFA::GNFA(DFA dfa)
      : finite_automaton(std::move(dfa).get_finite_automaton()) {
    for (auto const &[config, next_state] : dfa.get_transition_function()) {
      auto node = std::make_shared<regex::basic_node>(config.input_symbol);
      auto &regex_syntax_node = transition_function[{config.state, next_state}];
      if (!regex_syntax_node) {
        regex_syntax_node = std::move(node);
      } else {
        regex_syntax_node =
            std::make_shared<regex::union_node>(regex_syntax_node, node);
      }
    }
    auto new_start_state = add_new_state();
    transition_function[{new_start_state, get_start_state()}] =
        std::make_shared<regex::epsilon_node>();
    change_start_state(new_start_state);
    auto new_final_state = add_new_state();
    for (auto final_state : get_final_states()) {
      transition_function[{final_state, new_final_state}] =
          std::make_shared<regex::epsilon_node>();
    }
    replace_final_states(new_final_state);
  }

  std::shared_ptr<regex::syntax_node> GNFA::to_regex() {

    bool flag = true;
    while (flag) {
      flag = false;
      for (auto s : get_states()) {
        if (s != get_start_state() && !final_states.contains(s)) {
          remove_state(s);
          flag = true;
          break;
        }
      }
    }
    return transition_function.begin()->second;
  }

  void GNFA::remove_state(state_type removed_state) {
    cyy::computation::finite_automaton::remove_state(removed_state);
    auto old_transition_function = transition_function;

    for (auto from_state : get_states()) {
      if (is_final_state(from_state)) {
        continue;
      }
      for (auto to_state : get_states()) {
        if (to_state == get_start_state()) {
          continue;
        }
        auto it1 = old_transition_function.find({from_state, to_state});
        auto it2 = old_transition_function.find({from_state, removed_state});
        auto it3 = old_transition_function.find({removed_state, removed_state});
        auto it4 = old_transition_function.find({removed_state, to_state});

        std::shared_ptr<regex::syntax_node> from_to_regex_expr =
            std::make_shared<regex::union_node>(
                (it1 != old_transition_function.end()
                     ? it1->second
                     : std::make_shared<regex::empty_set_node>()),
                std::make_shared<regex::concat_node>(
                    (it2 != old_transition_function.end()
                         ? it2->second
                         : std::make_shared<regex::empty_set_node>()),
                    std::make_shared<regex::concat_node>(
                        std::make_shared<regex::kleene_closure_node>(
                            (it3 != old_transition_function.end()
                                 ? it3->second
                                 : std::make_shared<regex::empty_set_node>())

                                ),

                        (it4 != old_transition_function.end()
                             ? it4->second
                             : std::make_shared<regex::empty_set_node>())))

            );
        auto new_from_to_regex_expr = from_to_regex_expr->simplify();
        if (new_from_to_regex_expr) {
          from_to_regex_expr = new_from_to_regex_expr;
        }
        transition_function[{from_state, to_state}] = from_to_regex_expr;
      }
    }
    return;
  }
} // namespace cyy::computation
