/*!
 * \file turing_machine.hpp
 *
 */

#pragma once

#include "exception.hpp"
#include "multi_tape_turing_machine_base.hpp"

namespace cyy::computation {
  template <size_t tape_number>
  class multi_tape_Turing_machine
      : public multi_tape_Turing_machine_base<tape_number> {
  public:
    using situation_type =
        typename multi_tape_Turing_machine_base<tape_number>::situation_type;
    using action_type =
        typename multi_tape_Turing_machine_base<tape_number>::action_type;
    using  configuration_type=
        typename multi_tape_Turing_machine_base<tape_number>::configuration_type;
    using tape_type = typename multi_tape_Turing_machine_base<
        tape_number>::tape_type;
    using __transition_function_type =
        std::unordered_map<situation_type, action_type, tape_type>;

    class transition_function_type : public __transition_function_type {
    public:
      using __transition_function_type::__transition_function_type;
    };

    multi_tape_Turing_machine(finite_automaton finite_automaton_,
                              Turing_machine_base::state_type reject_state_,
                              ALPHABET_ptr tape_alphabet_,
                              transition_function_type transition_function_)
        : multi_tape_Turing_machine_base<tape_number>(std::move(finite_automaton_),
                                         reject_state_, tape_alphabet_),
          transition_function(std::move(transition_function_))

    {
      for (auto const &[situation, action] : transition_function) {
        if (situation.state == this->accept_state ||
            situation.state == this->reject_state) {
          throw exception::no_turing_machine(
              "accept state and reject state don't need transition");
        }
      }
    }

    bool
    operator==(const multi_tape_Turing_machine &rhs) const noexcept = default;

    bool recognize(symbol_string_view view) const override {
      tape_type tape;
      tape.reserve(view.size());
      for (auto s : view) {
        tape.push_back(s);
      }
      configuration_type configuration(this->get_start_state(), std::move(tape));
      while (true) {
        if (configuration.state == this->accept_state) {
          break;
        }
        if (configuration.state == this->reject_state) {
          return false;
        }
        auto it = transition_function.find(configuration.get_situation());
        if (it != transition_function.end()) {
          return false;
        }
        configuration.go(it->second);
      }
      return true;
    }

  private:
    transition_function_type transition_function;
  };

} // namespace cyy::computation
