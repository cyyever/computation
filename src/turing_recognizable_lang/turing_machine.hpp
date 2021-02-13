/*!
 * \file PDA.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <string_view>
#include <unordered_map>

#include "automaton/automaton.hpp"
#include "exception.hpp"
#include "hash.hpp"
#include "lang/alphabet_with_blank_symbol.hpp"

namespace cyy::computation {

  class Turing_machine final : public finite_automaton {
  public:
    enum class head_direction : int { left = 0, right };
    using tape_symbol_type = symbol_type;
    using tape_type = std::vector<tape_symbol_type>;
    struct situation_type {
      situation_type(state_type state_, tape_symbol_type tape_symbol_)
          : state(state_), tape_symbol{tape_symbol_} {}
      bool operator==(const situation_type &) const noexcept = default;
      state_type state;
      tape_symbol_type tape_symbol;
    };
    struct action_type {
      action_type(state_type state_, tape_symbol_type tape_symbol_,
                  head_direction direction_)
          : state(state_), tape_symbol{tape_symbol_}, direction(direction_) {}
      bool operator==(const action_type &) const noexcept = default;
      state_type state;
      tape_symbol_type tape_symbol;
      head_direction direction;
    };

    struct situation_hash_type {
      std::size_t operator()(const situation_type &x) const noexcept {
        size_t seed = 0;
        boost::hash_combine(seed, std::hash<state_type>()(x.state));
        boost::hash_combine(seed, std::hash<tape_symbol_type>()(x.tape_symbol));
        return seed;
      }
    };

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
    bool recognize(symbol_string_view view) const;

  private:
    struct configuration_type {
      configuration_type(state_type state_, std::vector<tape_symbol_type> tape_)
          : state{state_}, tape{std::move(tape_)} {}

      bool
      go(const Turing_machine::transition_function_type &transition_function);

      state_type state{};
      tape_type tape;
      size_t head_location{0};
      bool operator==(const configuration_type &) const noexcept = default;
    };

    state_type accept_state{};
    state_type reject_state{};
    ALPHABET_ptr tape_alphabet;
    transition_function_type transition_function;
  };

} // namespace cyy::computation
