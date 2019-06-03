/*!
 * \file lang.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include "common_tokens.hpp"

namespace cyy::computation {
  void common_tokens::print_symbol(std::ostream &os, symbol_type symbol) const {
    switch (static_cast<common_token>(symbol)) {
      case common_token::ascii_char:
        os << "\"ascii char\"";
        return;
      case common_token::escape_sequence:
        os << "\"escape sequence\"";
        return;
      case common_token::id:
        os << "\"id\"";
        return;
      case common_token::whitespace:
        os << "\"whitespace\"";
        return;
      case common_token::digit:
        os << "\"digit\"";
        return;
      case common_token::number:
        os << "\"number\"";
        return;
      case common_token::INT:
        os << "\"int\"";
        return;
      case common_token::FLOAT:
        os << "\"float\"";
        return;
      case common_token::record:
        os << "\"record\"";
        return;
      case common_token::CLASS:
        os << "\"class\"";
        return;
      case common_token::__end:
        return;
    }
    os << '\'' << static_cast<char>(symbol) << '\'';
  }
} // namespace cyy::computation
