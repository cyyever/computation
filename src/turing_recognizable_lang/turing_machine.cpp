/*!
 * \file turing_machine.cpp
 *
 * \brief
 */

#include "turing_machine.hpp"
namespace cyy::computation {

  Turing_machine::Turing_machine(finite_automaton finite_automaton_,
                                 state_type reject_state_,
                                 ALPHABET_ptr tape_alphabet_,
                                 transition_function_type transition_function_)
      : single_tape_Turing_machine(std::move(finite_automaton_), reject_state_,
                                   tape_alphabet_),
        transition_function(std::move(transition_function_)) {

    for (auto const &[situation, _] : transition_function) {
      if (situation.state == accept_state || situation.state == reject_state) {
        throw exception::no_turing_machine(
            "accept state and reject state don't need transition");
      }
    }
  }
  bool Turing_machine::recognize(symbol_string_view view) const {
    tape_type tape;
    tape.reserve(view.size());
    for (auto s : view) {
      tape.push_back(s);
    }
    configuration_type configuration(get_start_state(), std::move(tape));
    while (true) {
      if (configuration.state == accept_state) {
        break;
      }
      if (configuration.state == reject_state) {
        return false;
      }
      go(configuration);
    }
    return true;
  }

  void Turing_machine::go(configuration_type &configuration) const {
    auto tape_symbol = configuration.get_tape_symbol();
    auto next_state = reject_state;
    auto next_tape_symbol = tape_symbol;
    auto direction = head_direction::right;
    auto it = transition_function.find({configuration.state, tape_symbol});
    if (it != transition_function.end()) {
      next_state = it->second.state;
      next_tape_symbol = it->second.tape_symbol;
      direction = it->second.direction;
    }
    configuration.state = next_state;
    configuration.set_tape_symbol(next_tape_symbol);
    configuration.move_head(direction);
  }

} // namespace cyy::computation
