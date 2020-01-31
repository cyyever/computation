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
  bool PDA::simulate(symbol_string_view view) const {
    std::vector<stack_node> stack;

    std::unordered_set<
        std::pair<std::optional<PDA::stack_node>, PDA::state_type>>
        configuration{{{}, start_state}};
    configuration = move(stack, std::move(configuration));
    for (auto const &symbol : view) {
      configuration = move(stack, configuration, symbol);
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

  std::unordered_set<std::pair<std::optional<PDA::stack_node>, PDA::state_type>>
  PDA::move(
      std::vector<stack_node> &stack,
      const std::unordered_set<std::pair<std::optional<stack_node>, state_type>>
          &configuration,
      input_symbol_type a) const {
    std::unordered_set<
        std::pair<std::optional<PDA::stack_node>, PDA::state_type>>
        direct_reachable;
    for (auto const &[top_node_opt, s] : configuration) {
      if (top_node_opt.has_value()) {
        const auto &top_node = *top_node_opt;
        auto it = transition_function.find({a, s, top_node.content});
        if (it != transition_function.end()) {
          for (auto const &[next_state, next_stack_symbol] : it->second) {
            auto cur_node_opt = top_node.pop();
            if (next_stack_symbol.has_value()) {
              if (cur_node_opt.has_value()) {
                cur_node_opt = cur_node_opt->push(*next_stack_symbol);
              } else {
                cur_node_opt = stack_node(*next_stack_symbol, &stack);
              }
            }
            direct_reachable.emplace(cur_node_opt, next_state);
          }
        }
      }
      auto it = transition_function.find({a, s, {}});
      if (it != transition_function.end()) {
        for (auto const &[next_state, next_stack_symbol] : it->second) {
          auto cur_node_opt = top_node_opt;
          if (next_stack_symbol.has_value()) {
            if (cur_node_opt.has_value()) {
              cur_node_opt = cur_node_opt->push(*next_stack_symbol);
            } else {
              cur_node_opt = stack_node(*next_stack_symbol, &stack);
            }
          }
          direct_reachable.emplace(cur_node_opt, next_state);
        }
      }
    }
    return move(stack, std::move(direct_reachable));
  }

  std::unordered_set<std::pair<std::optional<PDA::stack_node>, PDA::state_type>>
  PDA::move(
      std::vector<stack_node> &stack,
      const std::unordered_set<std::pair<std::optional<stack_node>, state_type>>
          &configuration) const {

    auto result = std::move(configuration);
    while (true) {
      decltype(result) new_result;

      for (auto const &[top_node_opt, s] : result) {
        if (top_node_opt.has_value()) {
          const auto &top_node = *top_node_opt;
          auto it = transition_function.find({{}, s, top_node.content});
          if (it != transition_function.end()) {
            for (auto const &[next_state, next_stack_symbol] : it->second) {
              auto cur_node_opt = top_node.pop();
              if (next_stack_symbol.has_value()) {
                if (cur_node_opt.has_value()) {
                  cur_node_opt = cur_node_opt->push(*next_stack_symbol);
                } else {
                  cur_node_opt = stack_node(*next_stack_symbol, &stack);
                }
              }
              new_result.emplace(cur_node_opt, next_state);
            }
          }
        }
        auto it = transition_function.find({{}, s, {}});
        if (it != transition_function.end()) {
          for (auto const &[next_state, next_stack_symbol] : it->second) {
            auto cur_node_opt = top_node_opt;
            if (next_stack_symbol.has_value()) {
              if (cur_node_opt.has_value()) {
                cur_node_opt = cur_node_opt->push(*next_stack_symbol);
              } else {
                cur_node_opt = stack_node(*next_stack_symbol, &stack);
              }
            }
            new_result.emplace(cur_node_opt, next_state);
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

} // namespace cyy::computation
