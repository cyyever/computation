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
      : finite_automaton(std::move(finite_automaton_)),
        reject_state(reject_state_),
        transition_function(std::move(transition_function_)) {
    if (get_final_states().size() != 1) {
      throw exception::no_turing_machine("must have a accept state");
    }
    accept_state = *get_final_states().begin();
    if (!has_state(reject_state)) {
      throw exception::no_turing_machine("invalid reject state");
    }

    if (get_alphabet().contain(ALPHABET::blank_symbol)) {
      throw exception::invalid_alphabet(
          "input alphabet must not contain the blank symbol");
    }
    if (!tape_alphabet_->contain(get_alphabet())) {
      throw exception::invalid_alphabet(
          "tape alphabet must contain input alphabet");
    }
    tape_alphabet =
        std::make_shared<alphabet_with_blank_symbol>(tape_alphabet_);

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
      if (!configuration.go(transition_function)) {
        return false;
      }
    }
    return true;
  }

  bool Turing_machine::configuration_type::go(
      const Turing_machine::transition_function_type &transition_function) {
    auto tape_symbol = ALPHABET::blank_symbol;
    if (head_location < tape.size()) {
      tape_symbol = tape[head_location];
    }
    auto it = transition_function.find({state, tape_symbol});
    if (it == transition_function.end()) {
      return false;
    }
    auto const &[next_state, new_tape_symbol, direction] = it->second;
    state = next_state;
    if (head_location >= tape.size()) {
      tape.resize(head_location + 1, ALPHABET::blank_symbol);
    }
    tape[head_location] = new_tape_symbol;
    if (direction == head_direction::right) {
      head_location += 1;
    } else {
      if (head_location > 0) {
        head_location -= 1;
      }
    }
    return true;
  }

} // namespace cyy::computation
