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

    for (auto const &[situation, action_set] : transition_function) {
      if (situation.state == accept_state || situation.state == reject_state) {
        throw exception::no_turing_machine(
            "accept state and reject state don't need transition");
      }
      for (auto const &action : action_set) {
        if (action.get_direction() == head_direction::stay_put) {
          throw exception::no_turing_machine(
              "nondeterministic Turing machines don't support stay_put");
        }
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

      auto it = transition_function.find(configuration.get_situation());
      if (it == transition_function.end()) {
        continue;
      }
      for (auto const &action : it->second) {
        if (action.state == reject_state) {
          continue;
        }

        configuration_type new_configuration(configuration);
        new_configuration.go(action);
        new_configuration_set.emplace(std::move(new_configuration));
      }
    }
    configuration_set = std::move(new_configuration_set);
  }
} // namespace cyy::computation
