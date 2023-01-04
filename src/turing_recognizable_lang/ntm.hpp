/*!
 * \file ntm.hpp
 *
 */

#pragma once

#include <unordered_map>
#include <unordered_set>

#include "single_tape_turing_machine.hpp"

namespace cyy::computation {
  class NTM final : public single_tape_Turing_machine {
  public:
    using action_set_type = std::unordered_set<action_type, action_hash_type>;
    using __transition_function_type =
        std::unordered_map<situation_type, action_set_type ,situation_hash_type>;

    class transition_function_type : public __transition_function_type {
    public:
      using __transition_function_type::__transition_function_type;
      void add_moving_left_transition(state_type from_state,
                                      symbol_type old_tape_symbol,
                                      state_type to_state,
                                      symbol_type new_tape_symbol) {
        operator[](situation_type{from_state, old_tape_symbol})
            .emplace(to_state, new_tape_symbol, head_direction::left);
      }
      void add_moving_right_transition(state_type from_state,
                                       symbol_type old_tape_symbol,
                                       state_type to_state,
                                       symbol_type new_tape_symbol) {
        operator[](situation_type{from_state, old_tape_symbol})
            .emplace(to_state, new_tape_symbol, head_direction::right);
      }
      void add_moving_left_transitions(state_type looping_state,
                                       const symbol_set_type &tape_symbols) {
        add_moving_transitions(looping_state, tape_symbols, looping_state,
                               head_direction::left);
      }
      void add_moving_right_transitions(state_type looping_state,
                                        const symbol_set_type &tape_symbols) {
        add_moving_transitions(looping_state, tape_symbols, looping_state,
                               head_direction::right);
      }
      void add_moving_left_transitions(state_type from_state,
                                       const symbol_set_type &tape_symbols,
                                       state_type to_state) {
        add_moving_transitions(from_state, tape_symbols, to_state,
                               head_direction::left);
      }
      void add_moving_right_transitions(state_type from_state,
                                        const symbol_set_type &tape_symbols,
                                        state_type to_state) {
        add_moving_transitions(from_state, tape_symbols, to_state,
                               head_direction::right);
      }

    private:
      void add_moving_transitions(state_type from_state,
                                  const symbol_set_type &tape_symbols,
                                  state_type to_state,
                                  head_direction direction) {
        for (auto const tape_symbol : tape_symbols) {
          operator[]({from_state, tape_symbol})
              .emplace(to_state, tape_symbol, direction);
        }
      }
    };

    NTM(finite_automaton finite_automaton_, state_type reject_state_,
        ALPHABET_ptr tape_alphabet_,
        transition_function_type transition_function_);

    bool operator==(const NTM &rhs) const noexcept = default;

    auto const &get_transition_function() const noexcept {
      return transition_function;
    }
    bool recognize(symbol_string_view view) const override;

  private:
    using configuration_set_type =
        std::unordered_set<configuration_type, configuration_hash_type>;
    void go(configuration_set_type &configuration_set) const;

  private:
    transition_function_type transition_function;
  };

} // namespace cyy::computation
