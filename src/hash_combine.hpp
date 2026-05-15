/*!
 * \file hash_combine.hpp
 */

#pragma once

#include "std_prelude.hpp"

namespace cyy::computation {
  template <typename T>
  inline void hash_combine(std::size_t &seed, const T &v) noexcept {
    seed ^= std::hash<T>{}(v) + 0x9e3779b97f4a7c15ULL + (seed << 12) +
            (seed >> 4);
  }
} // namespace cyy::computation
