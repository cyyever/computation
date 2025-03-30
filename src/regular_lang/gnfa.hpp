/*!
 * \file nfa.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include "dfa.hpp"
#include "regex.hpp"

namespace cyy::computation {

  class GNFA final : public finite_automaton {
  public:
    using transition_function_type =
        std::unordered_map<std::pair<state_type, state_type>,
                           std::shared_ptr<regex::syntax_node>>;
    explicit GNFA(DFA dfa);

    GNFA(const GNFA &) = default;
    GNFA &operator=(const GNFA &) = default;

    GNFA(GNFA &&) noexcept = default;
    GNFA &operator=(GNFA &&) noexcept = default;
    ~GNFA() = default;

    std::shared_ptr<regex::syntax_node> to_regex();

  private:
    void remove_state(state_type removed_state);

  private:
    transition_function_type transition_function;
  };

} // namespace cyy::computation
