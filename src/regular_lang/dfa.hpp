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
  DFA(const std::set<uint64_t> &states_, const std::string &alphabet_name,
      uint64_t start_state_,
      const std::map<std::pair<uint64_t, symbol_type>, uint64_t>
          &transition_table_,
      const std::set<uint64_t> &final_states_)
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

private:
  std::optional<uint64_t> move(uint64_t s, symbol_type a) const {
    auto it = transition_table.find({s, a});
    if (it != transition_table.end()) {
      return {it->second};
    }
    return {};
  }

private:
  std::map<std::pair<uint64_t, symbol_type>, uint64_t> transition_table;
};

} // namespace cyy::computation
