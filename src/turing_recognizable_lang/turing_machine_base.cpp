/*!
 * \file turing_machine_base.cpp
 *
 * \brief Contains common code for various types of Turing machine
 */

#include "turing_machine_base.hpp"

#include "alphabet/alphabet_with_blank_symbol.hpp"
namespace cyy::computation {

  Turing_machine_base::Turing_machine_base(finite_automaton finite_automaton_,
                                           state_type reject_state_,
                                           ALPHABET_ptr tape_alphabet_)
      : finite_automaton(std::move(finite_automaton_)),
        reject_state(reject_state_) {
    if (!has_state(reject_state)) {
      throw exception::no_turing_machine(
          "the reject state is not a valid state");
    }

    if (get_final_states().size() != 1) {
      throw exception::no_turing_machine("must have a single accept state");
    }
    accept_state = *get_final_states().begin();

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
  }
  Turing_machine_base::tape_type
  Turing_machine_base::create_tape(symbol_string_view view) {
    tape_type tape;
    tape.reserve(view.size());
    for (auto s : view) {
      tape.push_back(s);
    }
    return tape;
  }

} // namespace cyy::computation
