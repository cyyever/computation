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
  symbol_type get_epsilon() const noexcept override { return epsilon; }
  symbol_type get_endmarker() const noexcept override { return endmarker; }
  symbol_type get_unincluded_symbol() const noexcept override {
    return unincluded_symbol;
  }

  void print(std::ostream &os, symbol_type symbol) const override {
    if (symbol == get_epsilon()) {
      os << "'epsilon'";
    } else if (symbol == get_endmarker()) {
      os << "$";
    } else {
      os << '\'' << static_cast<char>(symbol) << '\'';
    }
    return;
  }

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
  static constexpr symbol_type epsilon{max_symbol + 1};
  static constexpr symbol_type endmarker{max_symbol + 2};
  static constexpr symbol_type unincluded_symbol{max_symbol + 3};
};
} // namespace cyy::computation
