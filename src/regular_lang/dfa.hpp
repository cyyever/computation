/*!
 * \file dfa.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <map>
#include <optional>
#include <set>
#include <string>

#include "automaton.hpp"

namespace cyy::computation {

  class DFA final : public finite_automaton {
  public:
    using transition_table_type =
        std::map<std::pair<symbol_type, state_type>, state_type>;
    DFA(const std::set<state_type> &states_, const std::string &alphabet_name,
        state_type start_state_, const transition_table_type &transition_table_,
        const std::set<state_type> &final_states_)
        : finite_automaton(states_, alphabet_name, start_state_, final_states_),
          transition_table(transition_table_) {}

    bool equivalent_with(const DFA &rhs);

    bool simulate(symbol_string_view view) const {
      auto s = start_state;

      for (auto const &symbol : view) {
        auto opt_res = move(s, symbol);
        if (!opt_res) {
          return false;
        }
        s = opt_res.value();
      }
      return contain_final_state({s});
    }

    DFA minimize() const;

    std::optional<state_type> move(state_type s, symbol_type a) const {
      auto it = transition_table.find({a, s});
      if (it != transition_table.end()) {
        return {it->second};
      }
      return {};
    }

  private:
    transition_table_type transition_table;
  };

} // namespace cyy::computation
