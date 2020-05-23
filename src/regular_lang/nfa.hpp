/*!
 * \file nfa.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <string_view>
#include <unordered_map>

#include "../hash.hpp"
#include "dfa.hpp"

namespace cyy::computation {

  class NFA final : public finite_automaton {
  public:
    using transition_function_type =
        std::unordered_map<configuration_type, state_set_type>;
    using epsilon_transition_function_type =
        std::unordered_map<state_type, state_set_type>;
    NFA(const state_set_type &states_, std::string_view alphabet_name,
        state_type start_state_,
        const transition_function_type &transition_function_,
        const state_set_type &final_states_,
        const epsilon_transition_function_type &epsilon_transition_function_ =
            {})
        : finite_automaton(states_, alphabet_name, start_state_, final_states_),
          transition_function(transition_function_),
          epsilon_transition_function(epsilon_transition_function_) {}

    bool operator==(const NFA &rhs) const {
      return (this == &rhs) ||
             (finite_automaton::operator==(rhs) &&
              transition_function == rhs.transition_function &&
              epsilon_transition_function == rhs.epsilon_transition_function);
    }

    void add_sub_NFA(NFA rhs) {
      if (*alphabet != *rhs.alphabet) {
        throw std::invalid_argument("sub NFA has different alphabet name");
      }

      states.merge(rhs.states);
      transition_function.merge(rhs.transition_function);
      epsilon_transition_function.merge(rhs.epsilon_transition_function);
      final_states.merge(rhs.final_states);

      epsilon_closures.clear();
    }

    using finite_automaton::add_final_states;
    using finite_automaton::add_new_state;
    using finite_automaton::change_final_states;
    using finite_automaton::change_start_state;

    auto get_transition_function() const noexcept -> auto const & {
      return transition_function;
    }

    auto get_epsilon_transition_function() const noexcept -> auto const & {
      return epsilon_transition_function;
    }
    void replace_epsilon_transition(state_type from_state,
                                    state_set_type end_states) {
      epsilon_transition_function[from_state].clear();
      add_epsilon_transition(from_state, std::move(end_states));
    }

    void add_epsilon_transition(state_type from_state,
                                state_set_type end_states) {
      if (!includes(end_states)) {
        for (auto const &state : end_states) {
          if (!states.contains(state)) {
            throw exception::unexisted_finite_automaton_state(
                std::to_string(state));
          }
        }
      }
      if (!states.contains(from_state)) {
        throw exception::unexisted_finite_automaton_state(
            std::to_string(from_state));
      }
      epsilon_transition_function[from_state].merge(end_states);
      epsilon_closures.clear();
    }

    bool simulate(symbol_string_view view) const;

    // use subset construction
    std::pair<DFA, std::unordered_map<state_type, state_set_type>>
    to_DFA_with_mapping() const;
    DFA to_DFA() const;

  private:
    state_set_type epsilon_closure(const state_set_type &T) const;
    const state_set_type &epsilon_closure(state_type s) const;

    state_set_type move(const state_set_type &T, symbol_type a) const;

  private:
    transition_function_type transition_function;
    epsilon_transition_function_type epsilon_transition_function;
    mutable std::unordered_map<state_type, state_set_type> epsilon_closures;
  };

} // namespace cyy::computation
