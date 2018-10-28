/*!
 * \file ascii.hpp
 *
 * \brief
 * \author cyy
 * \date 2018-03-31
 */
#pragma once

#include "lang.hpp"

namespace cyy::computation {

template <symbol_type min_symbol, symbol_type max_symbol>
class range_alphabet : public ALPHABET {
public:
  void foreach_symbol(
      const std::function<void(const symbol_type &)> &callback) const override {
    for (symbol_type i = min_symbol; i <= max_symbol; i++) {
      callback(i);
    }
  }
  bool contain(symbol_type s) const noexcept override {
    return s >= min_symbol && s <= max_symbol;
  }

  size_t size() const noexcept override { return max_symbol - min_symbol + 1; }

private:
  void print_symbol(std::ostream &os, symbol_type symbol) const override {
    os << '\'' << static_cast<char>(symbol) << '\'';
    return;
  }

  symbol_type get_max_symbol() const override { return max_symbol; }
  symbol_type get_min_symbol() const override { return min_symbol; }
};
} // namespace cyy::computation
