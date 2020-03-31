/*!
 * \file dpda.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include <cassert>
#include <iterator>
#include <range/v3/algorithm.hpp>
#include <range/v3/iterator/insert_iterators.hpp>
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
    return is_final_state(configuration.first);
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


  void DPDA::remove_unreachable_states() {
    state_set_type reachable_states{start_state};

    while (true) {
      state_set_type new_reachable_states;
      for (auto state : reachable_states) {
        auto it = transition_function.find(state);
        if (it == transition_function.end()) {
          continue;
        }
        auto const &state_transition_function = it->second;
        for (auto const &[_, v] : state_transition_function) {
          new_reachable_states.insert(v.first);
        }
      }
      if (new_reachable_states.empty()) {
        break;
      }

      state_set_type diff;
      ranges::set_difference(new_reachable_states, reachable_states,
                             ranges::inserter(diff, diff.begin()));
      if (diff.empty()) {
        break;
      }
      reachable_states.merge(std::move(diff));
    }

    state_set_type diff;
    ranges::set_difference(states, reachable_states,
                           ranges::inserter(diff, diff.begin()));

    for (auto s : diff) {
      states.erase(s);
      transition_function.erase(s);
    }
  }
} // namespace cyy::computation
