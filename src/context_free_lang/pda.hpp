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
    using input_symbol_type = symbol_type;
    using stack_symbol_type = symbol_type;
    using transition_function_type = std::map<
        std::tuple<std::optional<input_symbol_type>, state_type,
                   std::optional<stack_symbol_type>>,
        std::set<std::pair<state_type, std::optional<stack_symbol_type>>>>;

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

    struct stack_node final {
      stack_node(stack_symbol_type content_, std::vector<stack_node> *stack_,
                 const std::optional<size_t> &prev_index_ = {})
          : content{content_}, prev_index{prev_index_}, stack{stack_} {
        index = stack->size();
        stack->push_back(*this);
      }

      std::optional<stack_node> pop() const {
        /* valid = false; */
        /* (*stack)[index].valid = false; */
        /* while (!stack->empty() && !stack->back().valid) { */
        /*   stack->pop_back(); */
        /* } */
        if (prev_index.has_value()) {
          return (*stack)[*prev_index];
        }
        return {};
      }
      stack_node push(stack_symbol_type content_) const {
        return stack_node(content_, stack, index);
      }

      bool operator==(const stack_node &rhs) const {
        if (this == &rhs) {
          return true;
        }

        /* if (!valid || !rhs.valid) { */
        /*   return false; */
        /* } */

        if (stack != rhs.stack || content != rhs.content) {
          return false;
        }
        auto i = prev_index;
        auto j = prev_index;

        while (true) {
          if (!i.has_value() && !j.has_value()) {
            break;
          }
          if (!i.has_value() || !j.has_value()) {
            return false;
          }

          if ((*stack)[*i].content != (*stack)[*j].content) {
            return false;
          }
          if ((*stack)[*i].index == (*stack)[*j].index) {
            break;
          }
          i = (*stack)[*i].prev_index;
          j = (*stack)[*j].prev_index;
        }
        return true;
      }

      size_t index;
      stack_symbol_type content;
      std::optional<size_t> prev_index{};
      std::vector<stack_node> *stack{nullptr};
      bool valid{true};
    };

  private:
    std::unordered_set<std::pair<std::optional<stack_node>, state_type>>
    move(std::vector<stack_node> &stack,
         const std::unordered_set<
             std::pair<std::optional<stack_node>, state_type>> &configuration,
         input_symbol_type a) const;

    std::unordered_set<std::pair<std::optional<stack_node>, state_type>>
    move(std::vector<stack_node> &stack,
         const std::unordered_set<std::pair<std::optional<stack_node>,
                                            state_type>> &configuration) const;

  private:
    std::shared_ptr<ALPHABET> stack_alphabet;
    transition_function_type transition_function;
  };

} // namespace cyy::computation

namespace std {
  template <> struct hash<cyy::computation::PDA::stack_node> {
    size_t operator()(const cyy::computation::PDA::stack_node &x) const
        noexcept {
      return ::std::hash<decltype(x.content)>()(x.content);
    }
  };
  template <>
  struct hash<std::pair<std::optional<cyy::computation::PDA::stack_node>,
                        cyy::computation::PDA::state_type>> {
    size_t
    operator()(const std::pair<std::optional<cyy::computation::PDA::stack_node>,
                               cyy::computation::PDA::state_type> &x

    ) const noexcept {
      return ::std::hash<decltype(x.first)>()(x.first) ^
             ::std::hash<decltype(x.second)>()(x.second)

          ;
    }
  };

} // namespace std
