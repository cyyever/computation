/*!
 * \file hash.hpp
 *
 */

#pragma once

#include <functional>
#include <utility>

#include <boost/functional/hash.hpp>
#include <boost/container_hash/hash.hpp>

namespace std {
  template <class T1, class T2> struct hash<std::pair<T1, T2>> {
    std::size_t operator()(const std::pair<T1, T2> &x) const noexcept {
      return boost::hash<decltype(x)>()(x);
    }
  };
} // namespace std
