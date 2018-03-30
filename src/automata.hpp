/*!
 * \file automata.hpp
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

class NFA {
public:
  NFA(const std::set<uint64_t> &states_, const std::string &alphabet_name,
      uint64_t start_state_,
      const std::map<std::pair<uint64_t, symbol_type>, std::set<uint64_t>>
          &transition_table_,
      const std::set<uint64_t> &final_states_)
      : states(states_), start_state(start_state_), final_states(final_states_),
        transition_table(transition_table_) {

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
    alphabet = make_alphabet(alphabet_name);
  }

  auto get_states() const -> auto const & { return states; }
  auto get_alphabet() const -> auto const & { return *alphabet; }
  auto get_transition_table() const -> auto const & { return transition_table; }
  auto get_final_states() const -> auto const & { return final_states; }
  uint64_t get_start_state() const { return start_state; }

  bool contain_final_state(const std::set<uint64_t> &T) const {
    for (const auto &f : final_states) {
      if (T.count(f) == 1) {
        return true;
      }
    }
    return false;
  }
  std::set<uint64_t> epsilon_closure(const std::set<uint64_t> &T) const {
    auto stack = T;
    auto res = T;
    auto epsilon = alphabet->get_epsilon();
    while (!stack.empty()) {
      decltype(stack) next_stack;
      for (auto const &t : res) {
        auto it = transition_table.find({t, epsilon});
        if (it == transition_table.end()) {
          continue;
        }
        for (auto const &u : it->second) {
          if (res.count(u) == 0) {
            next_stack.insert(u);
            res.insert(u);
          }
        }
      }
      stack = std::move(next_stack);
    }
    return res;
  }

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

  bool simulate(symbol_type *str, size_t str_len) {
    auto s = epsilon_closure({start_state});

    for (size_t i = 0; i < str_len; i++) {
      s = move(s, str[i]);
    }
    return contain_final_state(s);
  }

protected:
  std::set<uint64_t> states;
  std::shared_ptr< ALPHABET > alphabet;
  uint64_t start_state;
  std::set<uint64_t> final_states;

private:
  std::map<std::pair<uint64_t, symbol_type>, std::set<uint64_t>>
      transition_table;
};

class DFA final : public NFA {
public:
  DFA(const std::set<uint64_t> &states_, const std::string &alphabet_name,
      uint64_t start_state_,
      const std::map<std::pair<uint64_t, symbol_type>, uint64_t>
          &transition_table_,
      const std::set<uint64_t> &final_states_)
      : NFA(states_, alphabet_name, start_state_, {}, final_states_),
        transition_table(transition_table_) {

    for (auto const &s : states) {
      alphabet->foreach_symbol([this, s](auto const &a) {
        if (transition_table.find({s, a}) == transition_table.end()) {
          throw std::invalid_argument(std::string("no transition for state ") +
                                      std::to_string(s) + " and symbol " +
                                      std::to_string(a));
        }
      });
    }
    if (transition_table.size() != states.size() * alphabet->size()) {
      throw std::invalid_argument("invalid transition table");
    }
  }

  uint64_t move(uint64_t s, symbol_type a) const {
    auto it = transition_table.find({s, a});
    if (it != transition_table.end()) {
      return it->second;
    }
    return alphabet->get_epsilon();
  }

  bool simulate(symbol_type *str, size_t str_len) {
    auto s = start_state;

    for (size_t i = 0; i < str_len; i++) {
      s = move(s, str[i]);
    }
    return contain_final_state({s});
  }

  DFA minimize() const;

private:
  std::map<std::pair<uint64_t, symbol_type>, uint64_t> transition_table;
};

DFA NFA_to_DFA(const NFA &nfa);
} // namespace cyy::lang
