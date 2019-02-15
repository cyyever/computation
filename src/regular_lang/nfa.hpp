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
#include <unordered_map>

#include "dfa.hpp"

namespace cyy::computation {

  class NFA final : public finite_automaton {
  public:
    using transition_table_type =
        std::map<std::pair<symbol_type, state_type>, std::set<state_type>>;
    NFA(const std::set<state_type> &states_, const std::string &alphabet_name,
        state_type start_state_, const transition_table_type &transition_table_,
        const std::set<state_type> &final_states_)
        : finite_automaton(states_, alphabet_name, start_state_, final_states_),
          transition_table(transition_table_) {}

    void add_sub_NFA(NFA rhs, bool add_epsilon_transition) {
      if (alphabet->get_name() != rhs.alphabet->get_name()) {
        throw std::runtime_error("sub NFA has different alphabet name");
      }

      states.merge(rhs.states);
      transition_table.merge(rhs.transition_table);
      final_states.merge(rhs.final_states);
      if (add_epsilon_transition) {
        auto epsilon = alphabet->get_epsilon();
        transition_table[{epsilon, start_state}].insert(rhs.start_state);
      }
      epsilon_closures.clear();
      active_symbols_opt.reset();
      inactive_symbols_opt.reset();
    }

    void replace_final_states(const std::set<state_type> &final_states_) {
      for (auto const &final_state : final_states_) {
        if (!states.count(final_state)) {
          throw std::invalid_argument(std::string("unexisted state ") +
                                      std::to_string(final_state));
        }
      }
      final_states = final_states_;
    }

    auto get_transition_table() const noexcept -> auto const & {
      return transition_table;
    }
    auto get_transition_table() noexcept -> auto & { return transition_table; }

    auto get_start_epsilon_closure() const -> auto {
      return epsilon_closure(start_state);
    }

    bool simulate(symbol_string_view view) const {
      auto s = get_start_epsilon_closure();
      for (auto const &symbol : view) {
        s = move(s, symbol);
      }
      return contain_final_state(s);
    }

    std::set<state_type> move(const std::set<state_type> &T,
                              symbol_type a) const;

    // use subset construction
    std::pair<DFA, std::unordered_map<state_type, std::set<state_type>>>
    to_DFA_with_mapping() const;
    DFA to_DFA() const;

    bool operator==(const NFA &rhs) const {
      return (this == &rhs) || (finite_automaton::operator==(rhs) &&
                                transition_table == rhs.transition_table);
    }

  private:
    std::set<state_type> epsilon_closure(const std::set<state_type> &T) const;
    const std::set<NFA::state_type> &epsilon_closure(state_type s) const;
    const std::set<symbol_type> &get_inactive_symbols() const;
    const std::set<symbol_type> &get_active_symbols() const;

  private:
    transition_table_type transition_table;
    mutable std::map<state_type, std::set<state_type>> epsilon_closures;
    mutable std::optional<std::set<symbol_type>> active_symbols_opt;
    mutable std::optional<std::set<symbol_type>> inactive_symbols_opt;
  };

} // namespace cyy::computation
