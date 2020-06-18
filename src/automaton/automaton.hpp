/*!
 * \file automaton.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <algorithm>
#include <boost/dynamic_bitset.hpp>
#include <map>
#include <optional>
#include <ranges>
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
    using state_bitset_type = boost::dynamic_bitset<>;
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
      if (!has_state(start_state)) {
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

    const finite_automaton &get_finite_automaton() const & { return *this; }

    finite_automaton get_finite_automaton() && {
      return finite_automaton(std::move(*this));
    }

    auto const get_states() const noexcept { return std::views::all(states); }
    auto const &get_state_set() const noexcept { return states; }
    auto const &get_alphabet() const noexcept { return *alphabet; }
    auto const get_final_states() const noexcept {
      return std::views::all(final_states);
    }
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

    void replace_final_states(state_type s) {
      change_final_states(std::initializer_list{s});
    }

  protected:
    void set_start_state(state_type s) noexcept {
      check_state(s);
      start_state = s;
    }
    state_type add_new_state() {
      state_type new_state = 0;
      if (!states.empty()) {
        new_state = *states.rbegin() + 1;
      }
      states.insert(new_state);
      return new_state;
    }
    bool add_new_state(state_type s) { return states.emplace(s).second; }

    void merge(finite_automaton &&rhs) {
      assert(start_state == rhs.start_state);
      states.merge(std::move(rhs.states));
      final_states.merge(std::move(rhs.final_states));
    }

    void change_start_state(state_type s) {
      check_state(s);
      start_state = s;
    }

    bool has_state(state_type s) const { return states.contains(s); }
    void check_state(state_type s) const {
      if (!has_state(s)) {
        throw cyy::computation::exception::no_finite_automaton(
            std::string("unexisted state ") + std::to_string(s));
      }
    }

    template <std::ranges::range U>
    void change_final_states(U new_final_states) {
      final_states.clear();
      for (auto s : new_final_states) {
        add_final_states(s);
      }
    }

    void add_final_states(state_type s) {
      check_state(s);
      final_states.insert(s);
    }

  protected:
    void mark_all_states_final() { final_states = states; }

    void remove_state(state_type s) {
      check_state(s);
      states.erase(s);
      final_states.erase(s);
    }

  protected:
    static state_bitset_type
    state_set_to_bitset(const state_set_type &all_state_set,
                        const state_set_type &state_set);
    state_bitset_type
    state_set_to_bitset(const state_set_type &state_set) const;
    bool state_biset_contains(const state_bitset_type &state_bitset,
                              state_type state) const;

    static state_set_type &
    get_epsilon_closure(state_set_map_type &epsilon_closures, state_type s,
                        const state_set_map_type &epsilon_transition_function);

  protected:
    std::shared_ptr<ALPHABET> alphabet;

  private:
    state_set_type states;
    state_type start_state;

  protected:
    state_set_type final_states;
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
