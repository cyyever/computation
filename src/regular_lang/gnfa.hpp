/*!
 * \file nfa.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <map>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

#include "dfa.hpp"
#include "regex.hpp"

namespace cyy::computation {

  class GNFA final : public finite_automaton {
  public:
    using transition_function_type =
        std::map<std::pair<state_type, state_type>,
                 std::shared_ptr<regex::syntax_node>>;
    explicit GNFA(const DFA &dfa);

    /*
    GNFA(const std::set<state_type> &states_, std::string_view alphabet_name,
         state_type start_state_,
         const transition_function_type &transition_function_,
         const std::set<state_type> &final_states_,
         const epsilon_transition_function_type &epsilon_transition_function_ =
             {})
        : finite_automaton(states_, alphabet_name, start_state_, final_states_),
          transition_function(transition_function_),
          epsilon_transition_function(epsilon_transition_function_) {}

    auto get_transition_function() const noexcept -> auto const & {
      return transition_function;
    }
    */

  private:
    transition_function_type transition_function;
  };

} // namespace cyy::computation
