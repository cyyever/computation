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

namespace cyy::computation {

  class NFA final : public finite_automaton {
  public:
    using transition_function_type =
        std::map<std::pair<symbol_type, state_type>, std::set<state_type>>;
    using epsilon_transition_function_type =
        std::map<state_type, std::set<state_type>>;
    NFA(const std::set<state_type> &states_, std::string_view alphabet_name,
        state_type start_state_,
        const transition_function_type &transition_function_,
        const std::set<state_type> &final_states_,
        const epsilon_transition_function_type &epsilon_transition_function_ =
            {})
        : finite_automaton(states_, alphabet_name, start_state_, final_states_),
          transition_function(transition_function_),
          epsilon_transition_function(epsilon_transition_function_) {}

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

    void add_final_states(std::set<state_type> final_states_) {
      for (auto const &final_state : final_states_) {
        if (!states.count(final_state)) {
          throw exception::unexisted_finite_automaton_state(
              std::to_string(final_state));
        }
      }
      final_states.merge(final_states_);
    }
    void replace_final_states(const std::set<state_type> &final_states_) {
      final_states.clear();
      add_final_states(final_states_);
    }

    auto get_transition_function() const noexcept -> auto const & {
      return transition_function;
    }

    auto get_epsilon_transition_function() const noexcept -> auto const & {
      return epsilon_transition_function;
    }
    void replace_epsilon_transition(state_type from_state,
                                    std::set<state_type> end_states) {
      epsilon_transition_function[from_state].clear();
      add_epsilon_transition(from_state, std::move(end_states));
    }

    void add_epsilon_transition(state_type from_state,
                                std::set<state_type> end_states) {
      if (!includes(end_states)) {
        for (auto const &state : end_states) {
          if (!states.count(state)) {
            throw exception::unexisted_finite_automaton_state(
                std::to_string(state));
          }
        }
      }
      if (!states.count(from_state)) {
        throw exception::unexisted_finite_automaton_state(
            std::to_string(from_state));
      }
      epsilon_transition_function[from_state].merge(end_states);
      epsilon_closures.clear();
    }

    bool simulate(symbol_string_view view) const;

    // use subset construction
    std::pair<DFA, std::unordered_map<state_type, std::set<state_type>>>
    to_DFA_with_mapping() const;
    DFA to_DFA() const;

    bool operator==(const NFA &rhs) const {
      return (this == &rhs) ||
             (finite_automaton::operator==(rhs) &&
              transition_function == rhs.transition_function &&
              epsilon_transition_function == rhs.epsilon_transition_function);
    }
    bool operator!=(const NFA &rhs) const { return !operator==(rhs); }

  private:
    std::set<state_type> epsilon_closure(const std::set<state_type> &T) const;
    const std::set<state_type> &epsilon_closure(state_type s) const;

    std::set<state_type> move(const std::set<state_type> &T,
                              symbol_type a) const;

  private:
    transition_function_type transition_function;
    epsilon_transition_function_type epsilon_transition_function;
    mutable std::map<state_type, std::set<state_type>> epsilon_closures;
  };

} // namespace cyy::computation
