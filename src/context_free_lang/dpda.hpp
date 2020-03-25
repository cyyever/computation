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
#include <tuple>
#include <unordered_map>
#include <unordered_set>

#include "../automaton/automaton.hpp"
#include "../exception.hpp"

namespace cyy::computation {

  class DPDA final : public finite_automaton {
  public:
    using input_symbol_type = symbol_type;
    using stack_symbol_type = symbol_type;
    using transition_function_type =
        std::map<std::tuple<std::optional<input_symbol_type>, state_type,
                            std::optional<stack_symbol_type>>,
                 std::pair<state_type, std::optional<stack_symbol_type>>>;

    DPDA(std::set<state_type> states_, std::string_view input_alphabet_name,
         std::string_view stack_alphabet_name, state_type start_state_,
         transition_function_type transition_function_,
         std::set<state_type> final_states_)
        : finite_automaton(std::move(states_), input_alphabet_name,
                           start_state_, std::move(final_states_)),
          stack_alphabet(
              ::cyy::computation::ALPHABET::get(stack_alphabet_name)),
          transition_function(std::move(transition_function_)) {

      for (auto state : states) {
        for (auto input_symbol : *ALPHABET::get(input_alphabet_name)) {

          for (auto stack_symbol : *ALPHABET::get(stack_alphabet_name)) {
            size_t cnt = 0;
            cnt += transition_function.count({{}, state, {}});
            cnt += transition_function.count({input_symbol, state, {}});
            cnt += transition_function.count({{}, state, stack_symbol});
            cnt +=
                transition_function.count({input_symbol, state, stack_symbol});
            if (cnt != 1) {
              throw exception::no_DPDA(
                  "some combinations of states and symbols lack next state");
            }
          }
        }
      }
    }

    bool operator==(const DPDA &rhs) const {
      return (this == &rhs) || (finite_automaton::operator==(rhs) &&
                                stack_alphabet == rhs.stack_alphabet &&
                                transition_function == rhs.transition_function);
    }
    bool operator!=(const DPDA &rhs) const { return !operator==(rhs); }

    bool simulate(symbol_string_view view) const;

  private:
    using configuration_type =
        std::pair<state_type, std::vector<stack_symbol_type>>;

  private:
    std::optional<configuration_type>
    move(configuration_type configuration) const;
    std::optional<configuration_type>
    move(configuration_type configuration, input_symbol_type a) const;

  private:
    std::shared_ptr<ALPHABET> stack_alphabet;
    transition_function_type transition_function;
  };

} // namespace cyy::computation
