/*!
 * \file automaton.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

#include <boost/dynamic_bitset.hpp>

#include "../hash.hpp"
#include "../lang/alphabet.hpp"
#include "../lang/symbol.hpp"

namespace cyy::computation {

  class finite_automaton {
  public:
    using state_type = uint64_t;
    using state_set_type = std::set<state_type>;
    using state_bitset_type = boost::dynamic_bitset<>;
    using state_set_map_type = std::unordered_map<state_type, state_set_type>;
    using state_set_product_type =
        std::unordered_map<std::pair<state_type, state_type>, state_type>;
    using input_symbol_type = symbol_type;
    using stack_symbol_type = symbol_type;
    struct situation_type {
      state_type state;
      input_symbol_type input_symbol;
      bool operator==(const situation_type &) const noexcept = default;
    };

    finite_automaton(state_set_type states_, ALPHABET_ptr alphabet_,
                     state_type start_state_, state_set_type final_states_)
        : alphabet(alphabet_), states(std::move(states_)),
          start_state(start_state_), final_states(std::move(final_states_)) {
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

    bool operator==(const finite_automaton &rhs) const = default;

    const finite_automaton &get_finite_automaton() const & { return *this; }

    finite_automaton &get_finite_automaton() && { return *this; }

    auto get_states() const noexcept { return std::ranges::views::all(states); }
    symbol_set_type get_state_symbol_set() const;

    auto const &get_state_set() const noexcept { return states; }
    auto const &get_alphabet() const noexcept { return *alphabet; }
    auto const &get_alphabet_ptr() const noexcept { return alphabet; }
    auto const &get_final_states() const noexcept { return final_states; }
    state_type get_start_state() const noexcept { return start_state; }
    state_type get_max_state() const { return *states.rbegin(); }

    void set_alphabet(ALPHABET_ptr alphabet_) { alphabet = alphabet_; }
    bool contain_final_state(const state_set_type &T) const {
      return has_intersection(final_states, T);
    }

    bool is_final_state(state_type final_state) const {
      return final_states.contains(final_state);
    }

    bool includes(const state_set_type &T) const {
      return std::ranges::includes(states, T);
    }

    void replace_final_states(state_type s) {
      change_final_states(std::initializer_list<state_type>{s});
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
        throw cyy::computation::exception::no_finite_automaton(
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

    state_set_product_type
    get_state_set_product(const state_set_type &another_state_set) const;

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

    state_set_type from_bitset(const state_bitset_type &bitset) const;
    bool state_bitset_contains(const state_bitset_type &state_bitset,
                               state_type state) const;

    static bool has_intersection(const state_set_type &a,
                                 const state_set_type &b);

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
