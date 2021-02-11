/*!
 * \file endmarked_dpda.hpp
 *
 */

#pragma once

#include "dpda.hpp"
#include "exception.hpp"

namespace cyy::computation {

  class endmarked_DPDA : public DPDA {
  public:
    endmarked_DPDA(finite_automaton finite_automaton_,
                   ALPHABET_ptr stack_alphabet_,
                   transition_function_type transition_function_)
        : DPDA(std::move(finite_automaton_), stack_alphabet_,
               std::move(transition_function_)) {
      if (!alphabet->contain(ALPHABET::endmarker)) {
        throw exception::no_endmarked_DPDA(
            "input alphabet doesn't contain endmarker");
      }
    }

    explicit endmarked_DPDA(DPDA dpda);

    DPDA to_DPDA() const;

    void prepare_DCFG_conversion();

  private:
    state_set_type get_accept_states() const;
    void normalize_transitions();

  private:
    bool transition_normalized{};
  };

} // namespace cyy::computation
