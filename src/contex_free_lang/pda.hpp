/*!
 * \file PDA.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <map>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../automaton/automaton.hpp"

namespace cyy::computation {

  class PDA final : public finite_automaton {
  public:
    using transition_function_type =
        std::map<std::tuple<std::optional<symbol_type>, state_type,
                            std::optional<symbol_type>>,
                 std::set<std::pair<state_type, std::optional<symbol_type>>>>;

    using epsilon_transition_function_type =
        std::map<state_type, std::set<state_type>>;
    PDA(const std::set<state_type> &states_,
        std::string_view input_alphabet_name,
        std::string_view stack_alphabet_name, state_type start_state_,
        const transition_function_type &transition_function_,
        const std::set<state_type> &final_states_)
        : finite_automaton(states_, input_alphabet_name, start_state_,
                           final_states_),
          stack_alphabet(
              ::cyy::computation::ALPHABET::get(stack_alphabet_name)),
          transition_function(transition_function_) {}

    bool operator==(const PDA &rhs) const {
      return (this == &rhs) || (finite_automaton::operator==(rhs) &&
                                stack_alphabet == rhs.stack_alphabet &&
                                transition_function == rhs.transition_function);
    }
    bool operator!=(const PDA &rhs) const { return !operator==(rhs); }

    auto get_transition_function() const noexcept -> auto const & {
      return transition_function;
    }

    bool simulate(symbol_string_view view) const;

  private:
    std::set<std::pair<std::vector<symbol_type>, state_type>>
    move(const std::set<std::pair<std::vector<symbol_type>, state_type>>
             &configuration,
         symbol_type a) const;

    std::set<std::pair<std::vector<symbol_type>, state_type>> move(
        std::set<std::pair<std::vector<symbol_type>, state_type>> configuration)
        const;

  private:
    std::shared_ptr<ALPHABET> stack_alphabet;
    transition_function_type transition_function;
  };

} // namespace cyy::computation
