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

#include "../automaton/automaton.hpp"

namespace cyy::computation {

  class DFA final : public finite_automaton {
  public:
    using transition_function_type =
        std::map<std::pair<symbol_type, state_type>, state_type>;
    DFA(const std::set<state_type> &states_, const std::string &alphabet_name,
        state_type start_state_,
        const transition_function_type &transition_function_,
        const std::set<state_type> &final_states_)
        : finite_automaton(states_, alphabet_name, start_state_, final_states_),
          transition_function(transition_function_) {

      if (transition_function.size() != alphabet->size() * states.size()) {
        throw exception::no_DFA(
            "some combinations of states and symbols lack next state");
      }
    }

    auto get_transition_function() const -> const auto & {
      return transition_function;
    }
    bool equivalent_with(const DFA &rhs) const;

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
    std::optional<state_type> move(state_type s, symbol_type a) const {
      auto it = transition_function.find({a, s});
      if (it != transition_function.end()) {
        return {it->second};
      }
      return {};
    }

  private:
    transition_function_type transition_function;
  };

} // namespace cyy::computation
