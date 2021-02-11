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
transition_function_type transition_function_
                                 )
      : finite_automaton(std::move(finite_automaton_)),
        reject_state(reject_state_),transition_function(std::move(transition_function_)) {
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
    if (tape_alphabet_->contain(get_alphabet())) {
      throw exception::invalid_alphabet(
          "tape alphabet must contain input alphabet");
    }
    tape_alphabet =
        std::make_shared<alphabet_with_blank_symbol>(tape_alphabet_);

  }
} // namespace cyy::computation
