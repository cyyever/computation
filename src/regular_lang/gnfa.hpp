/*!
 * \file nfa.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <map>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

#include "dfa.hpp"
#include "regex.hpp"

namespace cyy::computation {

  class GNFA final : public finite_automaton {
  public:
    using transition_function_type =
        std::map<std::pair<state_type, state_type>,
                 std::shared_ptr<regex::syntax_node>>;
    explicit GNFA(const DFA &dfa);

    GNFA &operator=(const GNFA &) = default;
    GNFA(GNFA &&) = default;
    GNFA &operator=(GNFA &&) = default;
    ~GNFA() = default;

    std::shared_ptr<regex::syntax_node>  to_regex() const;

  private:
    GNFA remove_state(state_type removed_state) const;

  private:
    transition_function_type transition_function;
  };

} // namespace cyy::computation
