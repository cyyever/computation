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
#include <unordered_map>

#include "../automaton/automaton.hpp"
#include "../hash.hpp"

namespace cyy::computation {

  class DFA final : public finite_automaton {
  public:
    using transition_function_type =
        std::unordered_map<std::pair<symbol_type, state_type>, state_type>;
    DFA(const state_set_type &states_, const std::string &alphabet_name,
        state_type start_state_, transition_function_type transition_function_,
        state_set_type final_states_)
        : finite_automaton(states_, alphabet_name, start_state_, final_states_),
          transition_function(std::move(transition_function_)) {

      if (transition_function.size() != alphabet->size() * states.size()) {
        throw exception::no_DFA(
            "some combinations of states and symbols lack next state");
      }
    }
    DFA(const DFA &) = default;
    DFA &operator=(const DFA &) = default;
    DFA(DFA &&) = default;
    DFA &operator=(DFA &&) = default;
    ~DFA() = default;

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

    std::optional<state_type> move(state_type s, symbol_type a) const {
      mark_live_states();

      auto it = transition_function.find({a, s});
      if (it != transition_function.end()) {
        if (live_states_opt.value().count(it->second)) {
          return {it->second};
        }
      }
      return {};
    }

  private:
    void mark_live_states() const;

  private:
    mutable std::optional<state_set_type> live_states_opt;
    transition_function_type transition_function;
  };

} // namespace cyy::computation
