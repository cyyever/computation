/*!
 * \file PDA.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "automaton/automaton.hpp"
#include "exception.hpp"
#include "hash.hpp"
#include "lang/alphabet_with_blank_symbol.hpp"

namespace cyy::computation {

  class Turing_machine final : public finite_automaton {
  public:
    enum class head_direction : int { left = 0, right };
    using tape_symbol_type = symbol_type;
    struct situation_type {
      situation_type(state_type state_, tape_symbol_type tape_symbol_)
          : state(state_), tape_symbol{tape_symbol_} {}
      bool operator==(const situation_type &) const noexcept = default;
      state_type state;
      tape_symbol_type tape_symbol;
    };
    struct  action_type{
       action_type(state_type state_, tape_symbol_type tape_symbol_,head_direction direction_)
          : state(state_), tape_symbol{tape_symbol_},direction(direction_) {}
      bool operator==(const  action_type&) const noexcept = default;
      state_type state;
      tape_symbol_type tape_symbol;
      head_direction direction;
    };

    struct situation_hash_type {
      std::size_t operator()(const situation_type &x) const noexcept {
        size_t seed = 0;
        boost::hash_combine(seed, std::hash<state_type>()(x.state));
        boost::hash_combine(
            seed, std::hash<tape_symbol_type>()(x.tape_symbol));
        return seed;
      }
    };


    using transition_function_type =
        std::unordered_map<situation_type, std::set<action_type>,
                           situation_hash_type>;

    Turing_machine(finite_automaton finite_automaton_, state_type reject_state_,
                   ALPHABET_ptr tape_alphabet_,transition_function_type transition_function_);

    bool operator==(const Turing_machine &rhs) const noexcept = default;

    auto const &get_transition_function() const noexcept {
      return transition_function;
    }

  private:
    state_type accept_state{};
    state_type reject_state{};
    ALPHABET_ptr tape_alphabet;
    transition_function_type transition_function;
  };

} // namespace cyy::computation
