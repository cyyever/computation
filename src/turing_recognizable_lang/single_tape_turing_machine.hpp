/*!
 * \file turing_machine.hpp
 *
 */

#pragma once

#include "exception.hpp"
#include "hash.hpp"
#include "turing_machine_base.hpp"

namespace cyy::computation {
  class single_tape_Turing_machine : public Turing_machine_base {
  public:
    struct situation_type {
      situation_type(state_type state_, tape_symbol_type tape_symbol_)
          : state(state_), tape_symbol{tape_symbol_} {}
      bool operator==(const situation_type &) const noexcept = default;
      state_type state;
      tape_symbol_type tape_symbol;
    };
    struct situation_hash_type {
      std::size_t operator()(const situation_type &x) const noexcept {
        size_t seed = 0;
        boost::hash_combine(seed, std::hash<state_type>()(x.state));
        boost::hash_combine(seed, std::hash<tape_symbol_type>()(x.tape_symbol));
        return seed;
      }
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
    struct action_hash_type {
      std::size_t operator()(const action_type &x) const noexcept {
        size_t seed = 0;
        boost::hash_combine(seed, std::hash<state_type>()(x.state));
        boost::hash_combine(seed, std::hash<tape_symbol_type>()(x.tape_symbol));
        boost::hash_combine(seed, std::hash<head_direction>()(x.direction));
        return seed;
      }
    };

    class configuration_type {
    public:
      configuration_type(state_type state_, tape_type tape_,
                         size_t head_direction_ = 0)
          : state{state_}, tape{std::move(tape_)},
            head_location(head_direction_) {}

      state_type state{};
      tape_symbol_type get_tape_symbol() const {
        auto tape_symbol = ALPHABET::blank_symbol;
        if (head_location < tape.size()) {
          tape_symbol = tape[head_location];
        }
        return tape_symbol;
      }
      void set_tape_symbol(tape_symbol_type tape_symbol) {
        if (head_location >= tape.size()) {
          tape.resize(head_location + 1, ALPHABET::blank_symbol);
        }
        tape[head_location] = tape_symbol;
      }

      void move_head(head_direction direction) {
        if (direction == head_direction::stay_put) {
          return;
        }
        if (direction == head_direction::right) {
          head_location += 1;
        } else if (head_location > 0) {
          head_location -= 1;
        }
      }
      std::size_t get_hash() const noexcept {
        size_t seed = 0;
        boost::hash_combine(seed, std::hash<state_type>()(state));
        boost::hash_combine(seed, std::hash<size_t>()(head_location));
        return seed;


      }

      bool operator==(const configuration_type &) const noexcept = default;

    private:
      tape_type tape;
      size_t head_location{};
    };

    struct  configuration_hash_type{
      std::size_t operator()(const configuration_type &x) const noexcept {
        return x.get_hash();
      }
    };

    using Turing_machine_base::Turing_machine_base;

  };

} // namespace cyy::computation
