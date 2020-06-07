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
      std::optional<input_symbol_type> input_symbol;
      std::optional<stack_symbol_type> stack_symbol;
      bool operator==(const situation_type &) const noexcept = default;
    };

    struct situation_hash_type {
      std::size_t operator()(const situation_type &x) const noexcept {
        size_t seed = 0;
        if (x.input_symbol) {
          boost::hash_combine(
              seed, std::hash<input_symbol_type>()(x.input_symbol.value()));
        }
        if (x.stack_symbol) {
          boost::hash_combine(
              seed, std::hash<stack_symbol_type>()(x.stack_symbol.value()));
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
    };

    using transition_function_type = std::unordered_map<
        state_type,
        std::unordered_map<situation_type, action_type, situation_hash_type>>;

    DPDA(state_set_type states_, std::string_view input_alphabet_name,
         std::string_view stack_alphabet_name, state_type start_state_,
         transition_function_type transition_function_,
         state_set_type final_states_)
        : finite_automaton(std::move(states_), input_alphabet_name,
                           start_state_, std::move(final_states_)),
          stack_alphabet(
              ::cyy::computation::ALPHABET::get(stack_alphabet_name)),
          transition_function(std::move(transition_function_)) {

      check_transition_fuction();
    }

    bool operator==(const DPDA &rhs) const = default;

    bool recognize(symbol_string_view view) const;

    void normalize();
    DPDA complement() const;

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

    std::pair<std::map<state_type, std::set<stack_symbol_type>>,
              std::map<state_type, std::set<stack_symbol_type>>>
    get_looping_situations() const;

    void check_transition_fuction(bool check_input_endmark = false,
                                  bool check_stack_endmark = false);

  protected:
    std::shared_ptr<ALPHABET> stack_alphabet;
    transition_function_type transition_function;
    bool has_normalized{false};
    std::optional<state_type> reject_state_opt;
  };

} // namespace cyy::computation
