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
#include <tuple>
#include <unordered_map>
#include <unordered_set>

#include "../automaton/automaton.hpp"
#include "../exception.hpp"

namespace cyy::computation {

  class PDA final : public finite_automaton {
  public:
    using input_symbol_type = symbol_type;
    using stack_symbol_type = symbol_type;
    using transition_function_type = std::map<
        std::tuple<std::optional<input_symbol_type>, state_type,
                   std::optional<stack_symbol_type>>,
        std::set<std::pair<state_type, std::optional<stack_symbol_type>>>>;

    PDA(std::set<state_type> states_, std::string_view input_alphabet_name,
        std::string_view stack_alphabet_name, state_type start_state_,
        transition_function_type transition_function_,
        std::set<state_type> final_states_)
        : finite_automaton(std::move(states_), input_alphabet_name,
                           start_state_, std::move(final_states_)),
          stack_alphabet(
              ::cyy::computation::ALPHABET::get(stack_alphabet_name)),
          transition_function(std::move(transition_function_)) {}

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

    void normalize_transitions();

  private:
    struct stack_node final {
      stack_node(stack_symbol_type content_, size_t index_,
                 std::vector<stack_node> *stack_, size_t prev_index_ = 0)
          : content{content_}, index{index_},
            prev_index{prev_index_}, stack{stack_} {}

      stack_node pop_and_push(const std::optional<stack_symbol_type> &s) const {
        auto prev_node = pop();
        return prev_node.push(s);
      }
      stack_node push(const std::optional<stack_symbol_type> &content_) const {
        if (!content_.has_value()) {
          return *this;
        }
        auto cur_size = stack->size();
        stack->emplace_back(*content_, cur_size, stack, index);
        return stack->back();
      }

      bool operator==(const stack_node &rhs) const {
        if (this == &rhs) {
          return true;
        }
        if (stack != rhs.stack) {
          throw std::logic_error("different stacks");
        }

        if (index == 0 && rhs.index == 0) {
          return true;
        }
        if (index == 0 || rhs.index == 0) {
          return false;
        }

        if (content != rhs.content) {
          return false;
        }
        auto i = prev_index;
        auto j = rhs.prev_index;

        while (i && j) {
          if (i == j) {
            return true;
          }
          if ((*stack)[i].content != (*stack)[j].content) {
            return false;
          }
          i = (*stack)[i].prev_index;
          j = (*stack)[j].prev_index;
        }
        return i == j;
      }
      stack_symbol_type content;
      size_t index;
      size_t prev_index{0};
      std::vector<stack_node> *stack{nullptr};

    private:
      stack_node pop() const {
        if (index == 0) {
          throw exception::pop_empty_stack("");
        }
        return (*stack)[prev_index];
      }
    };

    struct configuration_hash_type {
      size_t operator()(const std::pair<cyy::computation::PDA::stack_node,
                                        cyy::computation::PDA::state_type> &x

      ) const noexcept {
        return ::std::hash<decltype(x.first.content)>()(x.first.content) ^
               ::std::hash<decltype(x.second)>()(x.second);
      }
    };

    using configuration_set_type =
        std::unordered_set<std::pair<stack_node, state_type>,
                           configuration_hash_type>;

  private:
    configuration_set_type move(const configuration_set_type &configurations,
                                input_symbol_type a) const;

    configuration_set_type move(configuration_set_type configurations) const;

    std::set<stack_symbol_type> get_used_stack_symbols() const;
    void add_epsilon_transition(state_type from_state, state_type to_state);

  private:
    std::shared_ptr<ALPHABET> stack_alphabet;
    transition_function_type transition_function;
  };

} // namespace cyy::computation
