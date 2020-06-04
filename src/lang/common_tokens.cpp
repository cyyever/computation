/*!
 * \file lang.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include "common_tokens.hpp"

namespace cyy::computation {
  std::string common_tokens::to_string(symbol_type symbol) const {
    switch (static_cast<common_token>(symbol)) {
      case common_token::ascii_char:
        return "\"ascii char\"";
      case common_token::escape_sequence:
        return "\"escape sequence\"";
      case common_token::id:
        return "\"id\"";
      case common_token::whitespace:
        return "\"whitespace\"";
      case common_token::digit:
        return "\"digit\"";
      case common_token::number:
        return "\"number\"";
      case common_token::INT:
        return "\"int\"";
      case common_token::FLOAT:
        return "\"float\"";
      case common_token::record:
        return "\"record\"";
      case common_token::CLASS:
        return "\"class\"";
      case common_token::__end:
        return "";
    }
    return ALPHABET::to_string(symbol);
  }
  bool common_tokens::contains_ASCII() const { return true; }
} // namespace cyy::computation
