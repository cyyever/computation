/*!
 * \file automaton.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <ranges>
#include <set>
#include <string>
#include <unordered_map>

#include <boost/dynamic_bitset.hpp>
#include <cyy/algorithm/alphabet/alphabet.hpp>
#include <cyy/algorithm/hash.hpp>

#include "../exception.hpp"

namespace cyy::computation {

  /* using namespace cyy::algorithm; */
  class finite_automaton {
  public:
    using state_type = uint64_t;
    using state_bitset_type = boost::dynamic_bitset<>;
    using ALPHABET_ptr = cyy::algorithm::ALPHABET_ptr;
    using ALPHABET = cyy::algorithm::ALPHABET;
    using symbol_set_type = cyy::algorithm::symbol_set_type;
    using symbol_string_view = cyy::algorithm::symbol_string_view;
    class state_set_type : public std::set<state_type> {
    public:
      using std::set<state_type>::set;
      bool operator==(const state_set_type &rhs) const = default;
      [[nodiscard]] bool has_intersection(const state_set_type &rhs) const;
      [[nodiscard]] bool includes(const state_set_type &rhs) const {
        return std::ranges::includes(*this, rhs);
      }
    };
    using state_set_map_type = std::unordered_map<state_type, state_set_type>;
    using state_set_product_type =
        std::unordered_map<std::pair<state_type, state_type>, state_type>;
    using input_symbol_type = cyy::algorithm::symbol_type;
    using stack_symbol_type = cyy::algorithm::symbol_type;
    struct situation_type {
      state_type state;
      input_symbol_type input_symbol;
      bool operator==(const situation_type &) const noexcept = default;
    };

    finite_automaton(state_set_type states_,
                     cyy::algorithm::ALPHABET_ptr alphabet_,
                     state_type start_state_, state_set_type final_states_)
        : alphabet(std::move(alphabet_)), states(std::move(states_)),
          start_state(start_state_), final_states(std::move(final_states_)) {
      if (states.empty()) {
        throw cyy::computation::exception::no_finite_automaton("no state");
      }
      if (!has_state(start_state)) {
        throw cyy::computation::exception::no_finite_automaton(
            "no such start state");
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

    [[nodiscard]] const finite_automaton &
    get_finite_automaton() const & noexcept {
      return *this;
    }

    finite_automaton &get_finite_automaton() && noexcept { return *this; }

    [[nodiscard]] cyy::algorithm::symbol_set_type get_state_symbol_set() const;

    auto const &get_states() const noexcept { return states; }
    auto const &get_alphabet() const noexcept { return *alphabet; }
    auto const &get_alphabet_ptr() const noexcept { return alphabet; }
    auto const &get_final_states() const noexcept { return final_states; }
    auto get_non_final_states() const noexcept {
      state_set_type non_final_states;
      std::ranges::set_difference(
          states, final_states,
          std::insert_iterator(non_final_states, non_final_states.begin()));
      return non_final_states;
    }
    state_type get_start_state() const noexcept { return start_state; }

    void set_alphabet(cyy::algorithm::ALPHABET_ptr alphabet_) {
      alphabet = std::move(alphabet_);
    }
    [[nodiscard]] bool contain_final_state(const state_set_type &T) const {
      return final_states.has_intersection(T);
    }
    state_set_type final_state_intersection(const state_set_type &T) const {
      state_set_type result;
      std::ranges::set_intersection(
          final_states, T, std::insert_iterator(result, result.begin()));
      return result;
    }

    state_type get_max_state() const { return *states.rbegin(); }
    bool is_final_state(state_type state) const {
      return final_states.contains(state);
    }

    void replace_final_states(state_type s) {
      change_final_states(std::initializer_list<state_type>{s});
    }

    state_type add_new_state() {
      auto new_state = get_max_state() + 1;
      states.insert(new_state);
      return new_state;
    }
    bool add_new_state(state_type s) { return states.emplace(s).second; }

    template <std::ranges::range U> void add_new_states(U state_set) {
      for (auto s : state_set) {
        add_new_state(s);
      }
    }

    void change_start_state(state_type s) {
      check_state(s);
      start_state = s;
    }

    [[nodiscard]] bool has_state(state_type s) const {
      return states.contains(s);
    }
    void check_state(state_type s) const {
      if (!has_state(s)) {
        throw cyy::computation::exception::no_finite_automaton(
            std::string("unexisted state ") + std::to_string(s));
      }
    }
    void clear_final_states() { final_states.clear(); }
    void add_final_state(state_type s) {
      check_state(s);
      final_states.insert(s);
    }

    template <std::ranges::range U>
    void change_final_states(U new_final_states) {
      clear_final_states();
      add_final_states(new_final_states);
    }

  protected:
    template <std::ranges::range U> void add_final_states(U new_final_states) {
      for (auto s : new_final_states) {
        add_final_state(s);
      }
    }

    state_set_product_type
    get_state_set_product(const state_set_type &another_state_set) const;

    void mark_all_states_final() { final_states = states; }

    void remove_state(state_type s) {
      check_state(s);
      states.erase(s);
      final_states.erase(s);
    }

    [[nodiscard]] std::string MMA_draw() const;

    state_bitset_type get_bitset(const state_set_type &state_set) const;

    state_bitset_type get_bitset(uint64_t bitset_value) const;

    state_set_type from_bitset(const state_bitset_type &bitset) const;
    [[nodiscard]] bool
    state_bitset_contains(const state_bitset_type &state_bitset,
                          state_type state) const;

    cyy::algorithm::ALPHABET_ptr alphabet;

  private:
    state_set_type states;
    state_type start_state;

  protected:
    state_set_type final_states;
  };

} // namespace cyy::computation
