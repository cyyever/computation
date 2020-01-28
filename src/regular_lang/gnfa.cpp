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

    std::shared_ptr<regex::syntax_node> GNFA::to_regex() const {

		    bool flag=true;
	    while (flag) {
		    flag=false;
		    for(auto s: states) {
			    if (s != start_state and !final_states.count(s)==1) {
			    remove_state(s);
			    flag =true;
			    break;
			    }
		    }

	    }
		return transition_function[{from_state, to_state}];
    }

  GNFA GNFA::remove_state(state_type removed_state) {
	CNFA new_gnfg(*this);
	new_gnfg.states.erase(removed_state);
	new_gnfg.transition_function.clear()

    for (auto from_state : states) {
      if (from_state == removed_state || is_final_state(from_state)) {
        continue;
      }
      for (auto to_state : states) {
        if (to_state == removed_state || to_state == start_state) {
          continue;
        }
        auto &from_to_regex_expr =transition_function[{from_state, to_state}];
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
	new_gnfg.transition_function[{from_state,to_state}]=from_to_regex_expr;
      }
    }
return new_gnfg;
  }
} // namespace cyy::computation
