/*!
 * \file ascii.hpp
 *
 * \brief
 * \author cyy
 * \date 2018-03-31
 */
#pragma once

#include "alphabet.hpp"

namespace cyy::computation {

  class range_alphabet : public ALPHABET {
  public:
    range_alphabet(symbol_type min_symbol_, symbol_type max_symbol_,
                   std::string_view name_)
        : min_symbol{min_symbol_}, max_symbol{max_symbol_} {
      if (max_symbol < min_symbol) {
        throw exception::empty_alphabet("range is empty");
      }
      name = name_;
    }

    bool contain(symbol_type s) const noexcept override {
      return s >= min_symbol && s <= max_symbol;
    }

    size_t size() const noexcept override {
      return max_symbol - min_symbol + 1;
    }

  private:
    void print_symbol(std::ostream &os, symbol_type symbol) const override {
      os << '\'' << static_cast<char>(symbol) << '\'';
      return;
    }

    symbol_type get_max_symbol() const override { return max_symbol; }
    symbol_type get_min_symbol() const override { return min_symbol; }
    symbol_type get_symbol(size_t index) const noexcept override {
      return static_cast<symbol_type>(min_symbol + index);
    }

  private:
    symbol_type min_symbol;
    symbol_type max_symbol;
  };
} // namespace cyy::computation
