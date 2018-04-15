/*!
 * \file automaton.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include "lang.hpp"
#include <map>
#include <optional>
#include <set>
#include <string>

namespace cyy::lang {

class finite_automaton {
public:
  finite_automaton(const std::set<uint64_t> &states_,
                   const std::string &alphabet_name, uint64_t start_state_,
                   const std::set<uint64_t> &final_states_)
      : alphabet(::cyy::lang::ALPHABET::get(alphabet_name)), states(states_),
        start_state(start_state_), final_states(final_states_) {

    if (states.empty()) {
      throw std::invalid_argument("no state");
    }
    if (!states.count(start_state)) {
      throw std::invalid_argument("unexisted start state");
    }
    for (auto const &final_state : final_states) {
      if (!states.count(final_state)) {
        throw std::invalid_argument(std::string("unexisted start state ") +
                                    std::to_string(final_state));
      }
    }
  }

  auto get_states() const -> auto const & { return states; }
  auto get_alphabet() const -> auto const & { return *alphabet; }
  auto get_final_states() const -> auto const & { return final_states; }
  uint64_t get_start_state() const { return start_state; }

  bool operator==(const finite_automaton &rhs) const {
    return (this == &rhs) ||
           (alphabet == rhs.alphabet && states == rhs.states &&
            start_state == rhs.start_state && final_states == rhs.final_states);
  }

  bool contain_final_state(const std::set<uint64_t> &T) const {
    for (const auto &f : final_states) {
      if (T.count(f) == 1) {
        return true;
      }
    }
    return false;
  }

protected:
  std::shared_ptr<ALPHABET> alphabet;
  std::set<uint64_t> states;
  uint64_t start_state;
  std::set<uint64_t> final_states;
};

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
    if (alphabet->name() != rhs.alphabet->name()) {
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

  auto get_transition_table() const -> auto const & { return transition_table; }
  auto get_transition_table() -> auto & { return transition_table; }

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

} // namespace cyy::lang
