/*!
 * \file ascii.hpp
 *
 * \brief
 * \author cyy
 * \date 2018-03-31
 */
#pragma once

#include "lang.hpp"

namespace cyy::lang {

class ASCII final : public ALPHABET {
public:
  // enum class common_token:symbol_type {

  symbol_type get_epsilon() const override { return 256; }
  symbol_type get_endmarker() const override { return 257; }

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
    for (symbol_type i = 0; i < 256; i++) {
      callback(i);
    }
  }
  bool contain(symbol_type s) const override { return s < 256; }
  size_t size() const override { return 256; }
  std::string name() const override { return "ASCII"; }
};

} // namespace cyy::lang
