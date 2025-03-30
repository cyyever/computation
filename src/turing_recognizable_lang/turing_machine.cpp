/*!
 * \file turing_machine.cpp
 *
 * \brief
 */

#include "turing_machine.hpp"

#include "exception.hpp"
namespace cyy::computation {

  Turing_machine::Turing_machine(finite_automaton finite_automaton_,
                                 state_type reject_state_,
                                 ALPHABET_ptr tape_alphabet_,
                                 transition_function_type transition_function_)
      : single_tape_Turing_machine(std::move(finite_automaton_), reject_state_,
                                   std::move(tape_alphabet_)),
        transition_function(std::move(transition_function_)) {

    for (auto const &[situation, action] : transition_function) {
      if (situation.state == accept_state || situation.state == reject_state) {
        throw exception::no_turing_machine(
            "accept state and reject state don't need transition");
      }
      if (action.get_direction() == head_direction::stay_put) {
        throw exception::no_turing_machine(
            "nondeterministic Turing machines don't support stay_put");
      }
    }
  }
  bool Turing_machine::recognize(symbol_string_view view) const {
    configuration_type configuration(get_start_state(), create_tape(view));
    while (true) {
      if (configuration.state == accept_state) {
        break;
      }
      if (configuration.state == reject_state) {
        return false;
      }
      auto it = transition_function.find(configuration.get_situation());
      if (it == transition_function.end()) {
        return false;
      }
      configuration.go(it->second);
    }
    return true;
  }

} // namespace cyy::computation
