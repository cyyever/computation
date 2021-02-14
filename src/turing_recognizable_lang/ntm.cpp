/*!
 * \file ntm.cpp
 *
 * \brief
 */

#include "ntm.hpp"
namespace cyy::computation {

  NTM::NTM(finite_automaton finite_automaton_, state_type reject_state_,
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
  bool NTM::recognize(symbol_string_view view) const {
    tape_type tape;
    tape.reserve(view.size());
    for (auto s : view) {
      tape.push_back(s);
    }
    configuration_set_type configuration_set{
        configuration_type(get_start_state(), std::move(tape))};
    while (true) {
      if (std::ranges::any_of(configuration_set,
                              [=, this](auto const &configuration) {
                                return configuration.state == accept_state;
                              })) {
        break;
      }
      go(configuration_set);
      if (configuration_set.empty()) {
        return false;
      }
    }
    return true;
  }

  void NTM::go(configuration_set_type &configuration_set) const {
    configuration_set_type new_configuration_set;
    for (const auto &configuration : configuration_set) {

      auto tape_symbol = configuration.get_tape_symbol();
      auto it = transition_function.find({configuration.state, tape_symbol});
      if (it == transition_function.end()) {
        continue;
      }
      for (auto const &action : it->second) {
        if (action.state == reject_state) {
          continue;
        }

        configuration_type new_configuration(configuration);
        new_configuration.state = action.state;
        new_configuration.set_tape_symbol(action.tape_symbol);
        new_configuration.move_head(action.direction);
        new_configuration_set.emplace(std::move(new_configuration));
      }
    }
    configuration_set = std::move(new_configuration_set);
  }
} // namespace cyy::computation
