/*!
 * \file automaton.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include "lang.hpp"
#include <map>
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

protected:
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
      s = move(s, symbol);
    }
    return contain_final_state({s});
  }

  DFA minimize() const;

private:
  uint64_t move(uint64_t s, symbol_type a) const {
    auto it = transition_table.find({s, a});
    if (it != transition_table.end()) {
      return it->second;
    }
    return alphabet->get_epsilon();
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

  auto get_transition_table() const -> auto const & { return transition_table; }

  bool simulate(symbol_string_view view) const {
    auto s = epsilon_closure({start_state});
    for (auto a : s) {
      std::cout << "a=" << a << std::endl;
    }

    for (auto const &symbol : view) {
      s = move(s, symbol);
    }
    return contain_final_state(s);
  }

  // use subset construction
  DFA to_DFA() const;

  bool operator==(const NFA &rhs) const {
    return (this == &rhs) || (finite_automaton::operator==(rhs) &&
                              transition_table == rhs.transition_table);
  }

private:
  std::set<uint64_t> epsilon_closure(const std::set<uint64_t> &T) const;

  std::set<uint64_t> move(const std::set<uint64_t> &T, symbol_type a) const {
    std::set<uint64_t> direct_reachable;

    for (const auto &s : T) {
      auto it = transition_table.find({s, a});
      if (it != transition_table.end()) {
        direct_reachable.insert(it->second.begin(), it->second.end());
      }
    }

    return epsilon_closure(direct_reachable);
  }

private:
  std::map<std::pair<uint64_t, symbol_type>, std::set<uint64_t>>
      transition_table;
};

} // namespace cyy::lang
