/*!
 * \file util.hpp
 *
 * \brief util functions
 * \author cyy
 * \date 2019-02-15
 */
#pragma once

#include <unordered_map>

#include <concepts>
#include <ranges>

template <typename T, std::ranges::input_range U>
requires std::same_as<T, std::ranges::range_value_t<U>>

    auto topological_sort(std::unordered_map<T, U> graph) {
  std::unordered_map<T, size_t> degrees;
  for (auto const &[from_node, to_nodes] : graph) {
    degrees.try_emplace(from_node, 0);
    for (auto const &to_node : to_nodes) {
      degrees[to_node]++;
    }
  }

  std::vector<T> result;
  for (auto const &[node, degree] : degrees) {
    if (degree == 0) {
      result.push_back(node);
    }
  }

  for (size_t i = 0; i < result.size(); i++) {
    auto it = graph.find(result[i]);
    if (it != graph.end()) {
      for (const auto &to_node : it->second) {
        if (--degrees[to_node] == 0) {
          result.push_back(to_node);
        }
      }
      graph.erase(it);
    }
  }
  return std::pair{result, graph};
}
