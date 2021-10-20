/*!
 * \file dfa.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "automaton/automaton.hpp"

namespace cyy::computation {

  class DFA final : public finite_automaton {
  public:
    using transition_function_type =
        std::unordered_map<situation_type, state_type>;
    DFA(state_set_type states_, ALPHABET_ptr alphabet_, state_type start_state_,
        transition_function_type transition_function_,
        state_set_type final_states_)
        : finite_automaton(std::move(states_), alphabet_, start_state_,
                           std::move(final_states_)),
          transition_function(std::move(transition_function_)) {
      if (transition_function.size() !=
          alphabet->size() * get_states().size()) {
        throw exception::no_DFA(
            "some combinations of states and symbols lack next state");
      }
    }
    DFA(const DFA &) = default;
    DFA &operator=(const DFA &) = default;
    DFA(DFA &&) = default;
    DFA &operator=(DFA &&) = default;
    ~DFA() = default;

    const auto &get_transition_function() const { return transition_function; }
    bool equivalent_with(const DFA &rhs) const;

    bool recognize(symbol_string_view view) const {
      auto s = get_start_state();

      for (auto const &symbol : view) {
        auto opt_res = go(s, symbol);
        if (!opt_res) {
          return false;
        }
        s = *opt_res;
      }
      return contain_final_state({s});
    }

    std::optional<state_type> go(state_type s, symbol_type a) const {
      auto it = transition_function.find({s, a});
      if (it != transition_function.end()) {
        return {it->second};
      }
      return {};
    }

    const state_set_type &get_live_states() const {
      mark_live_states();
      return live_states_opt.value();
    }

    bool is_live_state(state_type s) const {
      return get_live_states().contains(s);
    }

    DFA minimize() const;

    // get the intersection of two DFAs
    DFA intersect(const DFA &rhs) const;
    DFA complement() const;
    [[nodiscard]] std::string MMA_draw() const;

  private:
    void mark_live_states() const;

  private:
    mutable std::optional<state_set_type> live_states_opt;
    transition_function_type transition_function;
  };

} // namespace cyy::computation
