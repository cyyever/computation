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
    stack.emplace_back(0, 0, &stack);

    std::unordered_set<std::pair<PDA::stack_node, PDA::state_type>>
        configuration{{stack[0], start_state}};
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

  std::unordered_set<std::pair<PDA::stack_node, PDA::state_type>>
  PDA::move(const std::unordered_set<std::pair<stack_node, state_type>>
                &configuration,
            input_symbol_type a) const {
    std::unordered_set<std::pair<PDA::stack_node, PDA::state_type>>
        direct_reachable;
    for (auto const &[top_node, s] : configuration) {
      if (top_node.index != 0) {
        auto it = transition_function.find({a, s, top_node.content});
        if (it != transition_function.end()) {
          for (auto const &[next_state, next_stack_symbol] : it->second) {
            auto new_top_node = top_node.pop_and_push(next_stack_symbol);
            direct_reachable.emplace(new_top_node, next_state);
          }
        }
      }
      auto it = transition_function.find({a, s, {}});
      if (it != transition_function.end()) {
        for (auto const &[next_state, next_stack_symbol] : it->second) {
          auto new_top_node = top_node.push(next_stack_symbol);
          direct_reachable.emplace(new_top_node, next_state);
        }
      }
    }
    return move(std::move(direct_reachable));
  }

  std::unordered_set<std::pair<PDA::stack_node, PDA::state_type>>
  PDA::move(std::unordered_set<std::pair<stack_node, state_type>> configuration)
      const {

    auto result = std::move(configuration);
    while (true) {
      decltype(result) new_result;

      for (auto const &[top_node, s] : result) {
        if (top_node.index != 0) {
          auto it = transition_function.find({{}, s, top_node.content});
          if (it != transition_function.end()) {
            for (auto const &[next_state, next_stack_symbol] : it->second) {
              auto new_top_node = top_node.pop_and_push(next_stack_symbol);
              new_result.emplace(new_top_node, next_state);
            }
          }
        }
        auto it = transition_function.find({{}, s, {}});
        if (it != transition_function.end()) {
          for (auto const &[next_state, next_stack_symbol] : it->second) {
            auto new_top_node = top_node.push(next_stack_symbol);
            new_result.emplace(new_top_node, next_state);
          }
        }
      }
      auto prev_size = result.size();
      result.merge(std::move(new_result));
      if (result.size() == prev_size) {
        break;
      }
    }

    return result;
  }
  void PDA::add_epsilon_transition(state_type from_state, state_type to_state) {
    transition_function[{{}, from_state, {}}] = {{to_state, {}}};
  }

  void PDA::normalize_transitions() {
    auto new_state = add_new_state();
    for (auto final_state : final_states) {
      add_epsilon_transition(final_state, new_state);
    }
    for (auto const used_stack_symbol : get_used_stack_symbols()) {
      transition_function[{{}, new_state, used_stack_symbol}] = {
          {new_state, {}}};
    }
    auto new_final_state = add_new_state();
    add_epsilon_transition(new_state, new_final_state);
    change_final_states({new_final_state});

    transition_function_type new_transition;
    auto new_stack_symbol = *stack_alphabet->begin();
    for (const auto &[k, v] : transition_function) {
      auto const &top_symbol = std::get<2>(k);
      for (const auto &next_step : v) {
        auto const &new_top_symbol = next_step.second;
        if (top_symbol.has_value() && new_top_symbol.has_value()) {
          auto next_state = states.size();
          states.insert(next_state);
          new_transition[k] = {{next_state, {}}};
          new_transition[{{}, next_state, {}}] = {
              {next_step.first, new_top_symbol}};
          continue;
        }
        if (!top_symbol.has_value() && !new_top_symbol.has_value()) {
          auto next_state = states.size();
          states.insert(next_state);
          new_transition[k] = {{next_state, new_stack_symbol}};
          new_transition[{{}, next_state, new_stack_symbol}] = {
              {next_step.first, {}}};
          continue;
        }
      }
    }
    transition_function.merge(std::move(new_transition));
  }
  std::set<PDA::stack_symbol_type> PDA::get_used_stack_symbols() const {
    std::set<PDA::stack_symbol_type> res;
    for (auto const &[k, v] : transition_function) {
      auto const &top_symbol = std::get<2>(k);
      if (top_symbol.has_value()) {
        res.insert(*top_symbol);
      }
      for (const auto &next_step : v) {
        auto const &new_top_symbol = next_step.second;
        if (new_top_symbol.has_value()) {
          res.insert(*new_top_symbol);
        }
      }
    }
    return res;
  }
} // namespace cyy::computation
