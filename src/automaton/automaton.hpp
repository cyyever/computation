/*!
 * \file automaton.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <algorithm>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../hash.hpp"
#include "../lang/alphabet.hpp"

namespace cyy::computation {

  class finite_automaton {
  public:
    using state_type = uint64_t;
    using state_set_type = std::set<state_type>;
    using state_set_map_type = std::unordered_map<state_type, state_set_type>;
    using input_symbol_type = symbol_type;
    using stack_symbol_type = symbol_type;
    struct situation_type {
      state_type state;
      input_symbol_type input_symbol;
      bool operator==(const situation_type &) const noexcept = default;
    };

    finite_automaton(state_set_type states_, std::string_view alphabet_name,
                     state_type start_state_, state_set_type final_states_)
        : alphabet(::cyy::computation::ALPHABET::get(alphabet_name)),
          states(std::move(states_)), start_state(start_state_),
          final_states(std::move(final_states_)) {

      if (states.empty()) {
        throw cyy::computation::exception::no_finite_automaton("no state");
      }
      if (!states.contains(start_state)) {
        throw cyy::computation::exception::no_finite_automaton(
            "unexisted start state");
      }
      for (auto const &final_state : final_states) {
        check_state(final_state);
      }
    }
    finite_automaton(const finite_automaton &) = default;
    finite_automaton &operator=(const finite_automaton &) = default;
    finite_automaton(finite_automaton &&) = default;
    finite_automaton &operator=(finite_automaton &&) = default;
    ~finite_automaton() = default;

    auto const &get_states() const noexcept { return states; }
    auto const &get_alphabet() const noexcept { return *alphabet; }
    auto const &get_final_states() const noexcept { return final_states; }
    state_type get_start_state() const noexcept { return start_state; }
    bool operator==(const finite_automaton &rhs) const = default;

    bool contain_final_state(const state_set_type &T) const {
      auto it = T.begin();
      auto it2 = final_states.begin();
      while (it != T.end() && it2 != final_states.end()) {
        if (*it == *it2) {
          return true;
        }
        if (*it < *it2) {
          it++;
        } else {
          it2++;
        }
      }
      return false;
    }

    bool is_final_state(state_type final_state) const {
      return final_states.contains(final_state);
    }

    bool includes(const state_set_type &T) const {
      return std::ranges::includes(states, T);
    }

  protected:
    state_type add_new_state() {
      state_type new_state = 0;
      if (!states.empty()) {
        new_state = *states.rbegin() + 1;
      }
      states.insert(new_state);
      return new_state;
    }
    bool add_new_state(state_type s) { return states.emplace(s).second; }

    void change_start_state(state_type s) {
      check_state(s);
      start_state = s;
    }
    void check_state(state_type s) const {
      if (!states.contains(s)) {
        throw cyy::computation::exception::no_finite_automaton(
            std::string("unexisted state ") + std::to_string(s));
      }
    }
    void change_final_states(const state_set_type &T) {
      final_states.clear();
      for (auto s : T) {
        add_final_states(s);
      }
    }

    void change_final_states(const std::initializer_list<state_type> &T) {
      final_states.clear();
      for (auto s : T) {
        add_final_states(s);
      }
    }

    void add_final_states(state_type s) {
      check_state(s);
      final_states.insert(s);
    }

    const state_set_type &get_epsilon_closure(
        state_type s,
        const state_set_map_type &epsilon_transition_function) const;

  protected:
    std::shared_ptr<ALPHABET> alphabet;
    state_set_type states;
    state_type start_state;
    state_set_type final_states;
    mutable state_set_map_type epsilon_closures;
  };

} // namespace cyy::computation

namespace std {
  template <> struct hash<cyy::computation::finite_automaton::situation_type> {
    std::size_t
    operator()(const cyy::computation::finite_automaton::situation_type &x)
        const noexcept {
      size_t seed = 0;

      boost::hash_combine(
          seed,
          std::hash<cyy::computation::finite_automaton::state_type>()(x.state));
      boost::hash_combine(
          seed,
          std::hash<cyy::computation::finite_automaton::input_symbol_type>()(
              x.input_symbol));
      return seed;
    }
  };
} // namespace std
