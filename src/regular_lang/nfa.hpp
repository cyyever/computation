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
  NFA(const std::set<uint64_t> &states_, const std::string &alphabet_name,
      uint64_t start_state_,
      const std::map<std::pair<uint64_t, symbol_type>, std::set<uint64_t>>
          &transition_table_,
      const std::set<uint64_t> &final_states_)
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
      transition_table[{start_state, epsilon}].insert(rhs.start_state);
    }
  }

  void replace_final_states(const std::set<uint64_t> &final_states_) {
    for (auto const &final_state : final_states_) {
      if (!states.count(final_state)) {
        throw std::invalid_argument(std::string("unexisted start state ") +
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
    return epsilon_closure({start_state});
  }

  bool simulate(symbol_string_view view) const {
    auto s = get_start_epsilon_closure();
    for (auto const &symbol : view) {
      s = move(s, symbol);
    }
    return contain_final_state(s);
  }

  std::set<uint64_t> move(const std::set<uint64_t> &T, symbol_type a) const;

  // use subset construction
  std::pair<DFA, std::unordered_map<uint64_t, std::set<uint64_t>>>
  to_DFA_with_mapping() const;
  DFA to_DFA() const;

  bool operator==(const NFA &rhs) const {
    return (this == &rhs) || (finite_automaton::operator==(rhs) &&
                              transition_table == rhs.transition_table);
  }

private:
  std::set<uint64_t> epsilon_closure(const std::set<uint64_t> &T) const;

private:
  std::map<std::pair<uint64_t, symbol_type>, std::set<uint64_t>>
      transition_table;
};

} // namespace cyy::computation
