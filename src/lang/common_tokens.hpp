/*!
 * \file lang.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */
#pragma once

#include "range_alphabet.hpp"

namespace cyy::computation {

enum class common_token : symbol_type {
  ascii_char = 256,
  escape_sequence,
  id,
};

class common_tokens final
    : public range_alphabet<0, static_cast<symbol_type>(common_token::id)> {

private:
  void print_symbol(std::ostream &os, symbol_type symbol) const override {
    switch (symbol) {
    case common_token::ascii_char:
      os << "\"ascii char\"";
      break;

    case common_token::escape_sequence:
      os << "\"escape sequence\"";
      break;
    case common_token::id:
      os << "\"id\"";
      break;
    }
  }
};

} // namespace cyy::computation
