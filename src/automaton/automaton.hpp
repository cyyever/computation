/*!
 * \file automaton.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <map>
#include <optional>
#include <range/v3/algorithm.hpp>
#include <set>
#include <string>
#include <string_view>

#include "../lang/alphabet.hpp"

namespace cyy::computation {

  class finite_automaton {
  public:
    using state_type = uint64_t;
    using state_set_type = std::set<state_type>;
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

    auto get_states() const noexcept -> auto const & { return states; }
    auto get_alphabet() const noexcept -> auto const & { return *alphabet; }
    auto get_final_states() const noexcept -> auto const & {
      return final_states;
    }
    state_type get_start_state() const noexcept { return start_state; }
    bool operator==(const finite_automaton &rhs) const {
      return (this == &rhs) ||
             (alphabet == rhs.alphabet && states == rhs.states &&
              start_state == rhs.start_state &&
              final_states == rhs.final_states);
    }

    bool contain_final_state(const state_set_type &T) const {
      auto it = T.begin();
      auto it2 = final_states.begin();
      while (it != T.end() && it2 != final_states.end()) {
        if (*it == *it2) {
          return true;
        } else if (*it < *it2) {
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
      return ranges::includes(states, T);
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

  protected:
    std::shared_ptr<ALPHABET> alphabet;
    state_set_type states;
    state_type start_state;
    state_set_type final_states;
  }; // namespace cyy::computation

} // namespace cyy::computation
