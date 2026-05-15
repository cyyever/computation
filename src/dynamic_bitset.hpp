/*!
 * \file dynamic_bitset.hpp
 */

#pragma once

#include "std_prelude.hpp"

namespace cyy::computation {
  class dynamic_bitset {
  public:
    dynamic_bitset() = default;
    explicit dynamic_bitset(std::size_t n)
        : blocks_((n + W - 1) / W, 0), nbits_(n) {}
    dynamic_bitset(std::size_t n, std::uint64_t initial_value)
        : blocks_((n + W - 1) / W, 0), nbits_(n) {
      if (n == 0) {
        return;
      }
      blocks_[0] = initial_value;
      if (n < W) {
        blocks_[0] &= (~std::uint64_t{0}) >> (W - n);
      }
    }

    void set(std::size_t pos) { blocks_[pos / W] |= std::uint64_t{1} << (pos % W); }
    [[nodiscard]] bool test(std::size_t pos) const {
      return ((blocks_[pos / W] >> (pos % W)) & std::uint64_t{1}) != 0;
    }
    [[nodiscard]] unsigned long to_ulong() const {
      return blocks_.empty() ? 0UL
                             : static_cast<unsigned long>(blocks_[0]);
    }
    [[nodiscard]] std::size_t size() const noexcept { return nbits_; }

    bool operator==(const dynamic_bitset &) const = default;

  private:
    static constexpr std::size_t W = 64;
    std::vector<std::uint64_t> blocks_;
    std::size_t nbits_ = 0;
  };
} // namespace cyy::computation
