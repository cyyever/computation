/*!
 * \file dpda.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include <cassert>
#include <iterator>
#include <vector>

#include "dpda.hpp"

namespace cyy::computation {
  bool DPDA::simulate(symbol_string_view view) const {
    configuration_type configuration{start_state, {}};

    for (auto const &symbol : view) {
      auto configuration_opt = move(configuration, symbol);
      if (configuration_opt) {
        configuration = std::move(configuration_opt.value());
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
    auto it = transition_function.find({{}, configuration.first, {}});
    if (it != transition_function.end()) {
      configuration.first = it->second.first;
      if (it->second.second.has_value()) {
        configuration.second.push_back(it->second.second.value());
      }
      return std::move(configuration);
    }
    if (configuration.second.empty()) {
      return {};
    }
    auto stack_top = configuration.second.back();

    it = transition_function.find({{}, configuration.first, stack_top});
    if (it != transition_function.end()) {
      configuration.first = it->second.first;
      configuration.second.pop_back();
      if (it->second.second.has_value()) {
        configuration.second.push_back(it->second.second.value());
      }
      return std::move(configuration);
    }
    return {};
  }

  std::optional<DPDA::configuration_type>
  DPDA::move(configuration_type configuration, input_symbol_type a) const {
    auto it = transition_function.find({a, configuration.first, {}});
    if (it != transition_function.end()) {
      configuration.first = it->second.first;
      if (it->second.second.has_value()) {
        configuration.second.push_back(it->second.second.value());
      }
      return std::move(configuration);
    }
    if (configuration.second.empty()) {
      return {};
    }
    auto stack_top = configuration.second.back();

    it = transition_function.find({a, configuration.first, stack_top});
    if (it != transition_function.end()) {
      configuration.first = it->second.first;
      configuration.second.pop_back();
      if (it->second.second.has_value()) {
        configuration.second.push_back(it->second.second.value());
      }
      return std::move(configuration);
    }
    return {};
  }

} // namespace cyy::computation
