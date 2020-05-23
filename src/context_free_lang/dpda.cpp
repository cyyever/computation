/*!
 * \file dpda.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include <algorithm>
#include <cassert>
#include <iterator>
#include <unordered_map>
#include <vector>

#include "dpda.hpp"

namespace cyy::computation {
  bool DPDA::simulate(symbol_string_view view) const {
    configuration_type configuration{start_state, {}};

    size_t i = 0;
    while (i < view.size()) {
      auto const &symbol = view[i];
      auto configuration_opt = move(configuration, symbol);
      if (configuration_opt) {
        configuration = std::move(configuration_opt.value());
        i++;
        continue;
      }
      configuration_opt = move(std::move(configuration));
      assert(configuration_opt.has_value());
      configuration = std::move(configuration_opt.value());
    }
    while (!is_final_state(configuration.first)) {
      auto configuration_opt = move(std::move(configuration));
      if (!configuration_opt) {
        return false;
      }
      configuration = std::move(configuration_opt.value());
    }
    return true;
  }

  std::optional<DPDA::configuration_type>
  DPDA::move(configuration_type configuration) const {
    auto state = configuration.first;

    auto it = transition_function.find(state);
    if (it == transition_function.end()) {
      return {};
    }
    auto const &state_transition_function = it->second;

    auto it2 = state_transition_function.find({{}, {}});
    if (it2 != state_transition_function.end()) {
      configuration.first = it2->second.first;
      if (it2->second.second.has_value()) {
        configuration.second.push_back(it2->second.second.value());
      }
      return configuration;
    }
    if (configuration.second.empty()) {
      return {};
    }
    auto stack_top = configuration.second.back();

    it2 = state_transition_function.find(
        {std::optional<input_symbol_type>(), stack_top});
    if (it2 != state_transition_function.end()) {
      configuration.first = it2->second.first;
      configuration.second.pop_back();
      if (it2->second.second.has_value()) {
        configuration.second.push_back(it2->second.second.value());
      }
      return configuration;
    }
    return {};
  }

  std::optional<DPDA::configuration_type>
  DPDA::move(configuration_type configuration, input_symbol_type a) const {
    auto state = configuration.first;
    auto it = transition_function.find(state);
    if (it == transition_function.end()) {
      return {};
    }
    auto const &state_transition_function = it->second;
    auto it2 = state_transition_function.find({a, {}});
    if (it2 != state_transition_function.end()) {
      configuration.first = it2->second.first;
      if (it2->second.second.has_value()) {
        configuration.second.push_back(it2->second.second.value());
      }
      return configuration;
    }
    if (configuration.second.empty()) {
      return {};
    }
    auto stack_top = configuration.second.back();

    it2 = state_transition_function.find({a, stack_top});
    if (it2 != state_transition_function.end()) {
      configuration.first = it2->second.first;
      configuration.second.pop_back();
      if (it2->second.second.has_value()) {
        configuration.second.push_back(it2->second.second.value());
      }
      return configuration;
    }
    return {};
  }
  void DPDA::normalize() {

    std::unordered_map<state_type, state_type> parallel_states;
    std::unordered_map<state_type, state_type> reversed_parallel_states;
    auto get_parallel_state = [&, this](state_type s) {
      auto it = parallel_states.find(s);
      if (it == parallel_states.end()) {
        auto parallel_state = add_new_state();
        add_final_states(parallel_state);
        it = parallel_states.emplace(s, parallel_state).first;
        reversed_parallel_states.emplace(parallel_state, s);
      }
      return it->second;
    };
    transition_function_type new_transitions;
    for (auto &[from_state, transfers] : transition_function) {
      for (auto &[configuration, action] : transfers) {
        if (configuration.first.has_value()) {
          continue;
        }
        auto new_action = action;
        new_action.first = get_parallel_state(action.first);
        new_transitions[get_parallel_state(from_state)].emplace(
            configuration, std::move(new_action));
        if (is_final_state(from_state)) {
          action.first = get_parallel_state(action.first);
        }
      }
    }

    for (auto const &[parallel_state, s] : reversed_parallel_states) {
      for (auto const &[configuration, action] : transition_function[s]) {
        if (configuration.first.has_value()) {
          new_transitions[parallel_state].emplace(configuration, action);
        }
      }
    }
    transition_function.merge(std::move(new_transitions));

    auto old_start_state = start_state;
    auto endmarker = stack_alphabet->get_endmarker();
    start_state = add_new_state();
    transition_function[start_state][{}] = {old_start_state, endmarker};

    auto new_reject_state = add_new_state();
    auto new_accept_state = add_new_state();
    add_final_states(new_accept_state);

    new_transitions.clear();
    for (auto &[from_state, transfers] : transition_function) {
      for (auto &[configuration, action] : transfers) {
        if (!configuration.second.has_value()) {
          continue;
        }
        auto new_configuration = configuration;
        new_configuration.second = endmarker;
        if (is_final_state(from_state) && !configuration.first.has_value()) {
          new_transitions[from_state][std::move(new_configuration)] = {
              new_accept_state, {}};
        } else {
          new_transitions[from_state][std::move(new_configuration)] = {
              new_reject_state, {}};
        }
      }
    }
    transition_function.merge(std::move(new_transitions));
    for (auto a : *alphabet) {
      transition_function[new_reject_state][{a, {}}] = {new_reject_state, {}};
      transition_function[new_accept_state][{a, {}}] = {new_reject_state, {}};
    }
  }

  std::vector<std::pair<DPDA::state_type, DPDA::stack_symbol_type>>
  DPDA::get_looping_situations() const {
    std::map<state_type, std::set<stack_symbol_type>> looping_situations;

    for (auto state : states) {
      auto &looping_situations_of_state = looping_situations[state];
      for (auto stack_symbol : *stack_alphabet) {
        looping_situations_of_state.insert(stack_symbol);
      }
    }

    for (const auto &[from_state, transfers] : transition_function) {
      for (const auto &[configuration, action] : transfers) {
        if (configuration.first.has_value()) {
          looping_situations.erase(from_state);
          break;
        }
      }
    }

    while (true) {
      bool flag = false;
      for (const auto &[from_state, transfers] : transition_function) {
        for (const auto &[configuration, action] : transfers) {
          if (configuration.first.has_value()) {
            break;
          }
          if (!configuration.second.has_value()) {
            continue;
          }
          auto cur_stack_symbol = configuration.second.value();
          if (!action.second.has_value()) {
            if (looping_situations[from_state].erase(cur_stack_symbol)) {
              flag = true;
            }
          }
          if (action.second.has_value()) {
            auto next_state = action.first;
            auto next_stack_symbol = action.second.value();
            if (!looping_situations.contains(next_state) ||
                !looping_situations[next_state].contains(next_stack_symbol)) {
              if (looping_situations[from_state].erase(cur_stack_symbol)) {
                flag = true;
              }
            }
          }
        }
      }
      if (!flag) {
        break;
      }
    }

    return {};
  }
} // namespace cyy::computation
