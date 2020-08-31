/*!
 * \file automata.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <boost/dynamic_bitset.hpp>
#include <memory>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../hash.hpp"
#include "../lang/alphabet.hpp"

namespace cyy::computation {

  class finite_automata {
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

    finite_automata(state_set_type states_, ALPHABET_ptr alphabet_,
                    state_type start_state_, state_set_type final_states_)
        : alphabet(alphabet_), states(std::move(states_)),
          start_state(start_state_), final_states(std::move(final_states_)) {
      if (states.empty()) {
        throw cyy::computation::exception::no_finite_automata("no state");
      }
      if (!has_state(start_state)) {
        throw cyy::computation::exception::no_finite_automata(
            "unexisted start state");
      }
      for (auto const &final_state : final_states) {
        check_state(final_state);
      }
    }

    finite_automata(const finite_automata &) = default;
    finite_automata &operator=(const finite_automata &) = default;
    finite_automata(finite_automata &&) = default;
    finite_automata &operator=(finite_automata &&) = default;
    ~finite_automata() = default;

    bool operator==(const finite_automata &rhs) const = default;

    const finite_automata &get_finite_automata() const & { return *this; }

    finite_automata get_finite_automata() && {
      return finite_automata(std::move(*this));
    }

    auto const get_states() const noexcept { return std::views::all(states); }
    auto const &get_state_set() const noexcept { return states; }
    auto const &get_alphabet() const noexcept { return *alphabet; }
    auto const &get_alphabet_ptr() const noexcept { return alphabet; }
    auto const &get_final_states() const noexcept { return final_states; }
    state_type get_start_state() const noexcept { return start_state; }
    state_type get_max_state() const { return *states.rbegin(); }

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

    state_type add_new_state() {
      auto new_state = *states.rbegin() + 1;
      states.insert(new_state);
      return new_state;
    }
    bool add_new_state(state_type s) { return states.emplace(s).second; }

    void add_new_states(state_set_type state_set) {
      states.merge(std::move(state_set));
    }
    template <std::ranges::range U> void add_new_states(U state_set) {
      for (auto s : state_set) {
        add_new_state(s);
      }
    }

    void change_start_state(state_type s) {
      check_state(s);
      start_state = s;
    }

    bool has_state(state_type s) const { return states.contains(s); }
    void check_state(state_type s) const {
      if (!has_state(s)) {
        throw cyy::computation::exception::no_finite_automata(
            std::string("unexisted state ") + std::to_string(s));
      }
    }

    void clear_final_states() { final_states.clear(); }

    template <std::ranges::range U>
    void change_final_states(U new_final_states) {
      clear_final_states();
      add_final_states(new_final_states);
    }

    void add_final_state(state_type s) {
      check_state(s);
      final_states.insert(s);
    }
    template <std::ranges::range U> void add_final_states(U new_final_states) {
      for (auto s : new_final_states) {
        add_final_state(s);
      }
    }

  protected:
    void mark_all_states_final() { final_states = states; }

    void remove_state(state_type s) {
      check_state(s);
      states.erase(s);
      final_states.erase(s);
    }

    std::string MMA_draw() const;

  protected:
    state_bitset_type get_bitset(const state_set_type &state_set) const;

    state_bitset_type get_bitset(uint64_t bitset_value) const;

    bool state_bitset_contains(const state_bitset_type &state_bitset,
                               state_type state) const;

    static state_set_type &
    get_epsilon_closure(state_set_map_type &epsilon_closures, state_type s,
                        const state_set_map_type &epsilon_transition_function);

  protected:
    ALPHABET_ptr alphabet;

  private:
    state_set_type states;
    state_type start_state;

  protected:
    state_set_type final_states;
  };

} // namespace cyy::computation

namespace std {
  template <> struct hash<cyy::computation::finite_automata::situation_type> {
    std::size_t
    operator()(const cyy::computation::finite_automata::situation_type &x)
        const noexcept {
      size_t seed = 0;

      boost::hash_combine(
          seed,
          std::hash<cyy::computation::finite_automata::state_type>()(x.state));
      boost::hash_combine(
          seed,
          std::hash<cyy::computation::finite_automata::input_symbol_type>()(
              x.input_symbol));
      return seed;
    }
  };
} // namespace std
