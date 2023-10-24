/*!
 * \file turing_machine.hpp
 *
 * \brief Contains common code for various types of Turing machine
 */

#pragma once

#include <string_view>
#include <unordered_map>

#include "automaton/automaton.hpp"
#include "exception.hpp"

namespace cyy::computation {
  class Turing_machine_base : public finite_automaton {
  public:
    enum class head_direction : int { left = 0, right, stay_put };
    using tape_symbol_type = stack_symbol_type;
    using tape_type = std::vector<tape_symbol_type>;

    Turing_machine_base(finite_automaton finite_automaton_,
                        state_type reject_state_, ALPHABET_ptr tape_alphabet_);

    virtual ~Turing_machine_base() = default;

    bool operator==(const Turing_machine_base &rhs) const noexcept = default;

    virtual bool recognize(symbol_string_view view) const = 0;

  protected:
    static tape_type create_tape(symbol_string_view view);

    state_type accept_state{};
    state_type reject_state{};
    ALPHABET_ptr tape_alphabet;
  };

} // namespace cyy::computation
