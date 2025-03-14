/*!
 * \file nfa.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <unordered_map>

#include <boost/bimap.hpp>

#include "dfa.hpp"

namespace cyy::computation {

  class NFA final : public finite_automaton {
  public:
    using transition_function_type =
        std::unordered_map<situation_type, state_set_type>;
    using epsilon_transition_function_type = state_set_map_type;
    NFA(state_set_type states_, ALPHABET_ptr alphabet_, state_type start_state_,
        transition_function_type transition_function_,
        state_set_type final_states_,
        epsilon_transition_function_type epsilon_transition_function_ = {})
        : finite_automaton(std::move(states_), std::move(alphabet_),
                           start_state_, std::move(final_states_)),
          transition_function(std::move(transition_function_)),
          epsilon_transition_function(std::move(epsilon_transition_function_)) {
    }
    NFA(finite_automaton automaton,
        transition_function_type transition_function_,
        epsilon_transition_function_type epsilon_transition_function_ = {})
        : finite_automaton(std::move(automaton)),
          transition_function(std::move(transition_function_)),
          epsilon_transition_function(std::move(epsilon_transition_function_)) {
    }

    explicit NFA(DFA dfa) : NFA(std::move(dfa).get_finite_automaton(), {}) {
      for (const auto &[situation, next_state] :
           dfa.get_transition_function()) {
        transition_function[situation] = {next_state};
      }
    }

    bool operator==(const NFA &rhs) const {
      return finite_automaton::operator==(rhs) &&
             transition_function == rhs.transition_function &&
             epsilon_transition_function == rhs.epsilon_transition_function;
    }

    void add_sub_NFA(NFA rhs) {
      if (*alphabet != *rhs.alphabet) {
        throw std::invalid_argument("sub NFA has different alphabet name");
      }
      add_new_states(rhs.get_states());
      add_final_states(rhs.final_states);

      for (auto &[k, v] : rhs.transition_function) {
        transition_function[k].insert_range(std::move(v));
      }
      for (auto &[from_state, to_state_set] : rhs.epsilon_transition_function) {
        epsilon_transition_function[from_state].insert_range(std::move(to_state_set));
      }
    }

    auto const &get_transition_function() const noexcept {
      return transition_function;
    }

    auto const &get_epsilon_transition_function() const noexcept {
      return epsilon_transition_function;
    }
    void replace_epsilon_transition(state_type from_state,
                                    state_set_type end_states) {
      epsilon_transition_function[from_state].clear();
      add_epsilon_transition(from_state, std::move(end_states));
    }

    void add_transition(const situation_type &situation,
                        state_set_type end_states) {
      if (!has_state(situation.state)) {
        throw exception::unexisted_finite_automaton_state(
            std::to_string(situation.state));
      }
      for (auto const &state : end_states) {
        if (!has_state(state)) {
          throw exception::unexisted_finite_automaton_state(
              std::to_string(state));
        }
      }
      transition_function[situation].insert_range(end_states);
    }

    void add_epsilon_transition(state_type from_state,
                                state_set_type end_states) {
      if (!has_state(from_state)) {
        throw exception::unexisted_finite_automaton_state(
            std::to_string(from_state));
      }
      for (auto const &state : end_states) {
        if (!has_state(state)) {
          throw exception::unexisted_finite_automaton_state(
              std::to_string(state));
        }
      }
      epsilon_transition_function[from_state].insert_range(end_states);
      epsilon_closures.clear();
      epsilon_closure_refresh.clear();
    }

    bool recognize(symbol_string_view view) const;

    // use subset construction
    std::pair<DFA, boost::bimap<state_set_type, state_type>>
    to_DFA_with_mapping() const;
    DFA to_DFA() const;

    [[nodiscard]] std::string MMA_draw() const;

    const state_set_type &get_start_set() const {
      return get_epsilon_closure(get_start_state());
    }
    state_set_type go(const state_set_type &T, input_symbol_type a) const;

  private:
    const state_set_type &get_epsilon_closure(state_type s) const;

    transition_function_type transition_function;
    epsilon_transition_function_type epsilon_transition_function;
    mutable state_set_map_type epsilon_closures;
    mutable state_set_type epsilon_closure_refresh;
  };

} // namespace cyy::computation
