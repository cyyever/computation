/*!
 * \file turing_machine.hpp
 *
 */

#pragma once

#include <array>

#include "exception.hpp"
#include "hash.hpp"
#include "turing_machine_base.hpp"

namespace cyy::computation {
  template <size_t tape_number>
  class multi_tape_Turing_machine_base : public Turing_machine_base {
    static_assert(tape_number > 0);

  public:
    using multi_tape_type = std::array<tape_type, tape_number>;
    using tape_symbol_column_type = std::array<tape_symbol_type, tape_number>;
    using direction_column_type = std::array<head_direction, tape_number>;
    struct situation_type {
      situation_type(state_type state_, tape_symbol_column_type tape_symbols_)
          : state(state_), tape_symbols{tape_symbols_} {}
      template <typename = std::enable_if_t<tape_number == 1>>
      situation_type(state_type state_, tape_symbol_type tape_symbol)
          : state(state_) {
        tape_symbols[0] = tape_symbol;
      }
      bool operator==(const situation_type &) const noexcept = default;
      state_type state;
      tape_symbol_column_type tape_symbols;
    };
    struct situation_hash_type {
      std::size_t operator()(const situation_type &x) const noexcept {
        size_t seed = 0;
        boost::hash_combine(seed, std::hash<state_type>()(x.state));
        boost::hash_combine(
            seed, std::hash<tape_symbol_column_type>()(x.tape_symbols));
        return seed;
      }
    };
    struct action_type {
      action_type(state_type state_, tape_symbol_column_type tape_symbols_,
                  direction_column_type directions_)
          : state(state_), tape_symbols{tape_symbols_},
            directions(directions_) {}

      template <typename = std::enable_if_t<tape_number == 1>>
      action_type(state_type state_, tape_symbol_type tape_symbol,
                  head_direction direction)
          : state(state_) {
        tape_symbols[0] = tape_symbol;
        directions[0] = direction;
      }

      bool operator==(const action_type &) const noexcept = default;

      template <typename = std::enable_if_t<tape_number == 1>>
      head_direction get_direction() const {
        return directions[0];
      }

      state_type state;
      tape_symbol_column_type tape_symbols;
      direction_column_type directions;
    };
    struct action_hash_type {
      std::size_t operator()(const action_type &x) const noexcept {
        size_t seed = 0;
        boost::hash_combine(seed, std::hash<state_type>()(x.state));
        boost::hash_combine(
            seed, std::hash<tape_symbol_column_type>()(x.tape_symbols));
        boost::hash_combine(seed,
                            std::hash<direction_column_type>()(x.directions));
        return seed;
      }
    };

    class configuration_type {
    public:
      configuration_type(state_type state_, tape_type tape) : state{state_} {
        tapes[0] = std::move(tape);
      }

      situation_type get_situation() const {
        tape_symbol_column_type tape_symbol_column;
        for (size_t i = 0; i < tape_number; i++) {
          tape_symbol_column[i] = get_tape_symbol(i);
        }
        return {state, std::move(tape_symbol_column)};
      }

      tape_symbol_type get_tape_symbol(size_t tape_idx) const {
        auto const &tape = tapes[tape_idx];
        auto const &head_location = head_locations[tape_idx];
        auto tape_symbol = ALPHABET::blank_symbol;
        if (head_location < tape.size()) {
          tape_symbol = tape[head_location];
        }
        return tape_symbol;
      }

      template <typename = std::enable_if_t<tape_number == 1>>
      tape_symbol_type get_tape_symbol() const {
        return get_tape_symbol(0);
      }
      void set_tape_symbol(tape_symbol_type tape_symbol, size_t tape_idx) {
        auto &tape = tapes[tape_idx];
        auto const &head_location = head_locations[tape_idx];
        if (head_location >= tape.size()) {
          tape.resize(head_location + 1, ALPHABET::blank_symbol);
        }
        tape[head_location] = tape_symbol;
      }

      void move_head(head_direction direction, size_t tape_idx) {
        if (direction == head_direction::stay_put) {
          return;
        }
        auto &head_location = head_locations[tape_idx];
        if (direction == head_direction::right) {
          head_location += 1;
        } else if (head_location > 0) {
          head_location -= 1;
        }
      }

      void go(const action_type &action) {
        state = action.state;
        for (size_t i = 0; i < tape_number; i++) {
          set_tape_symbol(action.tape_symbols[i], i);
          move_head(action.directions[i], i);
        }
      }
      std::size_t get_hash() const noexcept {
        size_t seed = 0;
        boost::hash_combine(seed, std::hash<state_type>()(state));
        for (size_t i = 0; i < tape_number; i++) {
          boost::hash_combine(seed, std::hash<size_t>()(head_locations[i]));
        }
        return seed;
      }

      bool operator==(const configuration_type &) const noexcept = default;

      state_type state{};

    private:
      multi_tape_type tapes;
      std::array<size_t, tape_number> head_locations{};
    };

    struct configuration_hash_type {
      std::size_t operator()(const configuration_type &x) const noexcept {
        return x.get_hash();
      }
    };
    using Turing_machine_base::Turing_machine_base;
  };

} // namespace cyy::computation
