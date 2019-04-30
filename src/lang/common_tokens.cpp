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
        break;
      case common_token::escape_sequence:
        os << "\"escape sequence\"";
        break;
      case common_token::id:
        os << "\"id\"";
        break;
      case common_token::whitespace:
        os << "\"whitespace\"";
        break;
      case common_token::digit:
        os << "\"digit\"";
        break;
      case common_token::number:
        os << "\"number\"";
        break;
      case common_token::INT:
        os << "\"int\"";
        break;
      case common_token::FLOAT:
        os << "\"float\"";
        break;
      case common_token::record:
        os << "\"record\"";
        break;
      case common_token::CLASS:
        os << "\"class\"";
        break;
      default:
        os << '\'' << static_cast<char>(symbol) << '\'';
        break;
    }
  }
} // namespace cyy::computation
