/*!
 * \file nfa.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include <cassert>
#include <iterator>
#include <vector>

#include "pda.hpp"

namespace cyy::computation {

  std::set<std::pair<std::vector<PDA::stack_symbol_type>, PDA::state_type>>
  PDA::move(
      const std::set<std::pair<std::vector<stack_symbol_type>, state_type>>
          &configuration,
      input_symbol_type a) const {

    std::set<std::pair<std::vector<stack_symbol_type>, PDA::state_type>>
        direct_reachable;
    for (auto const &[stack, s] : configuration) {
      if (!stack.empty()) {
        auto it = transition_function.find({a, s, stack.back()});
        if (it != transition_function.end()) {
          for (auto const &[next_state, next_stack_symbol] : it->second) {
            auto next_stack = stack;
            next_stack.pop_back();
            if (next_stack_symbol.has_value()) {
              next_stack.push_back(*next_stack_symbol);
            }
            direct_reachable.emplace(next_stack, next_state);
          }
        }
      }
      auto it =
          transition_function.find({a, s, std::optional<stack_symbol_type>{}});
      if (it != transition_function.end()) {
        for (auto const &[next_state, next_stack_symbol] : it->second) {
          auto next_stack = stack;
          if (next_stack_symbol.has_value()) {
            next_stack.push_back(*next_stack_symbol);
          }
          direct_reachable.emplace(next_stack, next_state);
        }
      }
    }
    return move(std::move(direct_reachable));
  }

  std::set<std::pair<std::vector<PDA::stack_symbol_type>, PDA::state_type>>
  PDA::move(std::set<std::pair<std::vector<stack_symbol_type>, state_type>>
                configuration) const {
    auto result = std::move(configuration);
    while (true) {
      decltype(result) new_result;

      for (auto const &[stack, s] : result) {
        if (!stack.empty()) {
          auto it = transition_function.find(
              {std::optional<input_symbol_type>{}, s, stack.back()});
          if (it != transition_function.end()) {
            for (auto const &[next_state, next_stack_symbol] : it->second) {
              auto next_stack = stack;
              next_stack.pop_back();
              if (next_stack_symbol.has_value()) {
                next_stack.push_back(*next_stack_symbol);
              }
              new_result.emplace(next_stack, next_state);
            }
          }
        }
        auto it =
            transition_function.find({std::optional<input_symbol_type>{}, s,
                                      std::optional<stack_symbol_type>{}});
        if (it != transition_function.end()) {
          for (auto const &[next_state, next_stack_symbol] : it->second) {
            auto next_stack = stack;
            if (next_stack_symbol.has_value()) {
              next_stack.push_back(*next_stack_symbol);
            }
            new_result.emplace(next_stack, next_state);
          }
        }
      }
      auto prev_size = result.size();
      result.merge(new_result);
      if (result.size() == prev_size) {
        break;
      }
    }

    return result;
  }
  bool PDA::simulate(symbol_string_view view) const {
    std::set<std::pair<std::vector<stack_symbol_type>, state_type>>
        configuration{{{}, start_state}};
    configuration = move(std::move(configuration));
    for (auto const &symbol : view) {
      configuration = move(configuration, symbol);
      if (configuration.empty()) {
        return false;
      }
    }
    for (auto const &[_, s] : configuration) {
      if (is_final_state(s)) {
        return true;
      }
    }
    return false;
  }

} // namespace cyy::computation
