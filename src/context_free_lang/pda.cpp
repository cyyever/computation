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

    configuration_set_type configurations{{start_state, stack[0]}};
    configurations = move(std::move(configurations));
    for (auto const &symbol : view) {
      configurations = move(configurations, symbol);
      if (configurations.empty()) {
        return false;
      }
    }
    for (auto const &[s, _] : configurations) {
      if (is_final_state(s)) {
        return true;
      }
    }
    return false;
  }

  PDA::configuration_set_type
  PDA::move(const configuration_set_type &configurations,
            input_symbol_type a) const {
    configuration_set_type direct_reachable;
    for (auto const &[s, top_node] : configurations) {
      if (top_node.index != 0) {
        auto it = transition_function.find({s, a, top_node.content});
        if (it != transition_function.end()) {
          for (auto const &[next_state, next_stack_symbol] : it->second) {
            auto new_top_node = top_node.pop_and_push(next_stack_symbol);
            direct_reachable.emplace(next_state, new_top_node);
          }
        }
      }
      auto it = transition_function.find({s, a});
      if (it != transition_function.end()) {
        for (auto const &[next_state, next_stack_symbol] : it->second) {
          auto new_top_node = top_node.push(next_stack_symbol);
          direct_reachable.emplace(next_state, new_top_node);
        }
      }
    }
    return move(std::move(direct_reachable));
  }

  PDA::configuration_set_type
  PDA::move(configuration_set_type configurations) const {
    auto result = std::move(configurations);
    while (true) {
      decltype(result) new_result;

      for (auto const &[s, top_node] : result) {
        if (top_node.index != 0) {
          auto it = transition_function.find({s, {}, top_node.content});
          if (it != transition_function.end()) {
            for (auto const &[next_state, next_stack_symbol] : it->second) {
              auto new_top_node = top_node.pop_and_push(next_stack_symbol);
              new_result.emplace(next_state, new_top_node);
            }
          }
        }
        auto it = transition_function.find({s});
        if (it != transition_function.end()) {
          for (auto const &[next_state, next_stack_symbol] : it->second) {
            auto new_top_node = top_node.push(next_stack_symbol);
            new_result.emplace(next_state, new_top_node);
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
    transition_function[{from_state}] = {{to_state}};
  }

  void PDA::normalize_transitions() {
    auto new_state = add_new_state();
    for (auto final_state : final_states) {
      add_epsilon_transition(final_state, new_state);
    }
    for (auto const used_stack_symbol : get_used_stack_symbols()) {
      transition_function[{new_state, {}, used_stack_symbol}] = {{new_state}};
    }
    auto new_final_state = add_new_state();
    add_epsilon_transition(new_state, new_final_state);
    change_final_states({new_final_state});

    transition_function_type new_transition;
    auto new_stack_symbol = stack_alphabet->front();
    for (const auto &[k, v] : transition_function) {
      auto const &top_symbol = k.stack_symbol;
      for (const auto &next_step : v) {
        auto const &new_top_symbol = next_step.stack_symbol;
        if (top_symbol.has_value() != new_top_symbol.has_value()) {
          continue;
        }
        if (top_symbol.has_value()) {
          auto next_state = states.size();
          states.insert(next_state);
          new_transition[k] = {{next_state}};
          new_transition[{next_state}] = {{next_step.state, new_top_symbol}};
          continue;
        }
        if (!top_symbol.has_value()) {
          auto next_state = states.size();
          states.insert(next_state);
          new_transition[k] = {{next_state, new_stack_symbol}};
          new_transition[{next_state, {}, new_stack_symbol}] = {
              {next_step.state}};
          continue;
        }
      }
    }

    for (auto &[k, v] : transition_function) {
      auto const &top_symbol = k.stack_symbol;
      std::erase_if(v, [&top_symbol](const auto e) {
        return e.stack_symbol.has_value() == top_symbol.has_value();
      });
    }
    std::erase_if(transition_function,
                  [](const auto p) { return p.second.empty(); });
    transition_function.merge(std::move(new_transition));
    assert(final_states.size() == 1);

#ifndef NDEBUG
    for (const auto &[k, v] : transition_function) {
      auto const &top_symbol = k.stack_symbol;
      for (const auto &next_step : v) {
        auto const &new_top_symbol = next_step.stack_symbol;
        assert(top_symbol.has_value() != new_top_symbol.has_value());
      }
    }
#endif
  }
  std::set<PDA::stack_symbol_type> PDA::get_used_stack_symbols() const {
    std::set<PDA::stack_symbol_type> res;
    for (auto const &[k, v] : transition_function) {
      auto const &top_symbol = k.stack_symbol;
      if (top_symbol.has_value()) {
        res.insert(*top_symbol);
      }
      for (const auto &next_step : v) {
        auto const &new_top_symbol = next_step.stack_symbol;
        if (new_top_symbol.has_value()) {
          res.insert(*new_top_symbol);
        }
      }
    }
    return res;
  }
} // namespace cyy::computation
