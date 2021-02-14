/*!
 * \file hash.hpp
 *
 */

#pragma once

#include <functional>
#include <utility>

#include <boost/functional/hash.hpp>
#include <boost/container_hash/hash.hpp>
#include <concepts/concepts.hpp>
#include <ranges>

namespace std {
  template <class T1, class T2> struct hash<std::pair<T1, T2>> {
    std::size_t operator()(const std::pair<T1, T2> &x) const noexcept {
      return boost::hash<decltype(x)>()(x);
    }
  };
  template <typename T> concept Hashable = requires(T a) {
    { std::hash<T>{}(a) }
    ->::concepts::convertible_to<std::size_t>;
  };

  template <std::ranges::input_range T>
  requires Hashable<std::ranges::range_value_t<T>> struct hash<T> {
    std::size_t operator()(const T &x) const noexcept {
      return boost::hash_range(std::begin(x), std::end(x));
    }
  };
} // namespace std
