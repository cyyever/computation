/*!
 * \file nfa.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include <cassert>
#include <iterator>
#include <range/v3/algorithm.hpp>
#include <range/v3/iterator/insert_iterators.hpp>
#include <vector>

#include "pda.hpp"

namespace cyy::computation {

  std::set<std::pair<std::vector<symbol_type>, PDA::state_type>>
  PDA::move(const std::set<std::pair<std::vector<symbol_type>, state_type>>
                &configuration,
            symbol_type a) const {

    std::set<std::pair<std::vector<symbol_type>, PDA::state_type>>
        direct_reachable;
    for (auto const &[stack, s] : configuration) {
      if (!stack.empty()) {
        auto it = transition_function.find({stack.back(), s, a});
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
      auto it = transition_function.find({std::optional<symbol_type>{}, s, a});
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

    auto result = std::move(direct_reachable);
    while (true) {
      decltype(result) new_result;

      for (auto const &[stack, s] : result) {
        if (!stack.empty()) {
          auto it = transition_function.find(
              {stack.back(), s, std::optional<symbol_type>{}});
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
        auto it = transition_function.find(
            {std::optional<symbol_type>{}, s, std::optional<symbol_type>{}});
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
  /* bool PDA::simulate(symbol_string_view view) const { */
  /*   auto s = epsilon_closure(start_state); */
  /*   for (auto const &symbol : view) { */
  /*     s = move(s, symbol); */
  /*     if (s.empty()) { */
  /*       return false; */
  /*     } */
  /*   } */
  /*   return contain_final_state(s); */
  /* } */

} // namespace cyy::computation
