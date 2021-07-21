/*!
 * \file DPDA.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../automaton/automaton.hpp"
#include "../exception.hpp"
#include "../hash.hpp"

namespace cyy::computation {

  class DPDA : public finite_automaton {
  public:
    struct situation_type {
      situation_type() = default;
      situation_type(input_symbol_type input_symbol_)
          : input_symbol{input_symbol_} {}
      situation_type(std::optional<input_symbol_type> input_symbol_,
                     stack_symbol_type stack_symbol_)
          : input_symbol{input_symbol_}, stack_symbol{stack_symbol_} {}
      bool operator==(const situation_type &) const noexcept = default;
      bool has_pop() const { return stack_symbol.has_value(); }
      stack_symbol_type get_poped_symbol() const {
        return stack_symbol.value();
      }
      bool use_input() const { return input_symbol.has_value(); }
      std::optional<input_symbol_type> input_symbol;
      std::optional<stack_symbol_type> stack_symbol;
    };

    struct situation_hash_type {
      std::size_t operator()(const situation_type &x) const noexcept {
        size_t seed = 0;
        if (x.input_symbol) {
          boost::hash_combine(
              seed, std::hash<input_symbol_type>()(*x.input_symbol);
        }
        if (x.stack_symbol) {
          boost::hash_combine(
              seed, std::hash<stack_symbol_type>()(*x.stack_symbol);
        }
        return seed;
      }
    };

    struct action_type {
      action_type() {}
      action_type(state_type state_) : state(state_) {}
      action_type(state_type state_,
                  std::optional<stack_symbol_type> stack_symbol_)
          : state(state_), stack_symbol(stack_symbol_) {}
      state_type state{};
      std::optional<stack_symbol_type> stack_symbol;
      bool operator==(const action_type &) const noexcept = default;
      auto operator<=>(const action_type &) const noexcept = default;
      bool has_push() const { return stack_symbol.has_value(); }
      stack_symbol_type get_pushed_symbol() const {
        return stack_symbol.value();
      }
    };

    using __transition_function_type = std::unordered_map<
        state_type,
        std::unordered_map<situation_type, action_type, situation_hash_type>>;
    class transition_function_type : public __transition_function_type {
    public:
      using __transition_function_type::__transition_function_type;

      void add_epsilon_transition(state_type from_state, action_type action) {
        auto &transfers = operator[](from_state);
        transfers.emplace(situation_type{}, std::move(action));
      }

      void pop_stack_and_action(state_type from_state, action_type action,
                                const ALPHABET &stack_alphabet_) {
        auto &transfers = operator[](from_state);
        for (auto stack_symbol : stack_alphabet_) {
          transfers[{{}, stack_symbol}] = action;
        }
      }

      void check_stack_and_action(state_type from_state,
                                  situation_type situation, action_type action,
                                  finite_automaton &automaton) {
        assert(situation.has_pop());
        auto &transfers = operator[](from_state);
        auto new_state = automaton.add_new_state();
        transfers[situation] = {new_state, situation.stack_symbol};

        auto &new_transfers = operator[](new_state);
        new_transfers[{}] = std::move(action);
      }
      void make_reject_state(state_type s, ALPHABET_ptr input_alphabet_) {
        auto &transfers = operator[](s);
        for (auto a : *input_alphabet_) {
          transfers[{a}] = {s};
        }
      }
    };

    DPDA(finite_automaton finite_automaton_, ALPHABET_ptr stack_alphabet_,
         transition_function_type transition_function_)
        : finite_automaton(std::move(finite_automaton_)),
          stack_alphabet(stack_alphabet_),
          transition_function(std::move(transition_function_)) {
      check_transition_fuction();
    }

    bool operator==(const DPDA &rhs) const = default;

    bool recognize(symbol_string_view view) const;

    void normalize();
    DPDA complement() const;
    auto const &get_transition_function() const { return transition_function; }

    [[nodiscard]] std::string MMA_draw() const;

  protected:
    struct configuration_type {
      state_type state;
      std::vector<stack_symbol_type> stack;
    };

  protected:
    std::optional<configuration_type>
    go(configuration_type configuration) const;
    std::optional<configuration_type> go(configuration_type configuration,
                                         input_symbol_type a) const;

    std::pair<std::map<state_type, symbol_set_type>,
              std::map<state_type, symbol_set_type>>
    get_looping_situations() const;

    void check_transition_fuction() const;

  protected:
    ALPHABET_ptr stack_alphabet;
    transition_function_type transition_function;
    bool has_normalized{false};
    std::optional<state_type> reject_state_opt;
  };

} // namespace cyy::computation
