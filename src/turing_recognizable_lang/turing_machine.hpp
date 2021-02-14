/*!
 * \file turing_machine.hpp
 *
 */

#pragma once

#include <string_view>
#include <unordered_map>

#include "exception.hpp"
#include "hash.hpp"
#include "lang/alphabet_with_blank_symbol.hpp"
#include "multi_tape_turing_machine.hpp"
#include "single_tape_turing_machine.hpp"

namespace cyy::computation {
  class Turing_machine final : public single_tape_Turing_machine {
  public:
    using __transition_function_type =
        std::unordered_map<situation_type, action_type, situation_hash_type>;

    class transition_function_type : public __transition_function_type {
    public:
      using __transition_function_type::__transition_function_type;
      void add_moving_left_transition(state_type from_state,
                                      symbol_type old_tape_symbol,
                                      state_type to_state,
                                      symbol_type new_tape_symbol) {
        try_emplace(
            situation_type{from_state, old_tape_symbol},
            action_type{to_state, new_tape_symbol, head_direction::left});
      }
      void add_moving_right_transition(state_type from_state,
                                       symbol_type old_tape_symbol,
                                       state_type to_state,
                                       symbol_type new_tape_symbol) {
        try_emplace(
            situation_type{from_state, old_tape_symbol},
            action_type{to_state, new_tape_symbol, head_direction::right});
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
          try_emplace(situation_type{from_state, tape_symbol},
                      action_type{to_state, tape_symbol, direction});
        }
      }
    };

    Turing_machine(finite_automaton finite_automaton_, state_type reject_state_,
                   ALPHABET_ptr tape_alphabet_,
                   transition_function_type transition_function_);

    bool operator==(const Turing_machine &rhs) const noexcept = default;

    auto const &get_transition_function() const noexcept {
      return transition_function;
    }
    bool recognize(symbol_string_view view) const override;

  private:
    void go(configuration_type &configuration) const;

  private:
    transition_function_type transition_function;
  };

} // namespace cyy::computation
