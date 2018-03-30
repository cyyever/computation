/*!
 * \file lang.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */
#pragma once

#include "lang.hpp"

namespace cyy::lang {

class common_tokens final : public ALPHABET {
public:
  //enum class common_token:symbol_type {
  enum token{
    epsilon=128,
    id,
    endmark,
  };

  symbol_type get_epsilon() const override { return token::epsilon; }
  symbol_type get_endmarker() const override { return token::endmark; }

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
    for (symbol_type i = 0; i <256; i++) {
      callback(i);
    }
    for (symbol_type i = token::epsilon+1; i <token::endmark; i++) {
      callback(i);
    }
  }
  bool contain(symbol_type s) const override { return s <256 || s>token::epsilon && s<token::endmark; }
  size_t size() const override { return 256+token::endmark-token::epsilon-1; }
  std::string name() const override { return "common_tokens"; }
};

} // namespace cyy::lang
