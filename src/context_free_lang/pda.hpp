/*!
 * \file PDA.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */
#pragma once

#include "../exception.hpp"
#include "../regular_lang/dfa.hpp"

namespace cyy::computation {

  class PDA final : public finite_automaton {
  public:
    struct situation_type {
      bool use_input() const { return input_symbol.has_value(); }
      auto get_input() const { return input_symbol.value(); }
      bool has_pop() const { return stack_symbol.has_value(); }
      auto get_stack_symbol() const { return stack_symbol.value(); }
      bool operator==(const situation_type &) const noexcept = default;
      state_type state{};
      std::optional<input_symbol_type> input_symbol{};
      std::optional<stack_symbol_type> stack_symbol{};
    };

    struct action_type {
      bool operator==(const action_type &) const noexcept = default;
      bool has_push() const { return stack_symbol.has_value(); }
      auto get_stack_symbol() const { return stack_symbol.value(); }
      state_type state{};
      std::optional<stack_symbol_type> stack_symbol{};
    };

    using transition_function_type =
        std::unordered_map<situation_type, std::unordered_set<action_type>>;

    PDA(finite_automaton finite_automaton_, ALPHABET_ptr stack_alphabet_,
        transition_function_type transition_function_)
        : finite_automaton(std::move(finite_automaton_)),
          stack_alphabet(stack_alphabet_),
          transition_function(std::move(transition_function_)) {}

    bool operator==(const PDA &rhs) const noexcept = default;

    auto const &get_transition_function() const noexcept {
      return transition_function;
    }

    bool recognize(symbol_string_view view) const;

    void prepare_CFG_conversion();

    [[nodiscard]] std::string MMA_draw() const;
    PDA intersect(const DFA &rhs) const;

  private:
    struct stack_node final {
      stack_node(stack_symbol_type content_, size_t index_,
                 std::vector<stack_node> *stack_, size_t prev_index_ = 0)
          : content{content_}, index{index_}, prev_index{prev_index_},
            stack{stack_} {}

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

    struct configuration_type {
      configuration_type(state_type state_, stack_node top_node_)
          : state{state_}, top_node{std::move(top_node_)} {}
      state_type state{};
      stack_node top_node;
      bool operator==(const configuration_type &) const noexcept = default;
    };
    struct configuration_hash_type {
      size_t operator()(const configuration_type &x) const noexcept {
        return ::std::hash<stack_symbol_type>()(x.top_node.content) ^
               ::std::hash<state_type>()(x.state);
      }
    };

    using configuration_set_type =
        std::unordered_set<configuration_type, configuration_hash_type>;

  private:
    configuration_set_type go(const configuration_set_type &configurations,
                              input_symbol_type a) const;

    configuration_set_type go(configuration_set_type configurations) const;

    symbol_set_type get_in_use_stack_symbols() const;
    void add_epsilon_transition(state_type from_state, state_type to_state);

  private:
    ALPHABET_ptr stack_alphabet;
    transition_function_type transition_function;
  };

} // namespace cyy::computation
