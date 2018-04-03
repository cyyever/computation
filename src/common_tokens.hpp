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
   enum class token:symbol_type {
    epsilon = 256,
    ascii_char,
    escape_sequence,
    id,
    endmarker,
  };

   symbol_type get_epsilon() const override { return static_cast<symbol_type>(token::epsilon); }
   symbol_type get_endmarker() const override { return static_cast<symbol_type>(token::endmarker); }

  void print(std::ostream &os, symbol_type symbol) const override {
	  switch(static_cast<token>(symbol)) {
	case	  token::epsilon:
      os << "epsilon";
      break;
	case token::endmarker:
      os << "$";
      break;
      case token::ascii_char:
      os << "'ascii char'";
      break;
      case token::escape_sequence:
      os << "'escape sequence'";
      break;
      default:
      os << '\'' << static_cast<char>(symbol) << '\'';
	  }


    return;
  }

  void foreach_symbol(
      const std::function<void(const symbol_type &)> &callback) const override {
    for (symbol_type i = 0; i < 256; i++) {
      callback(i);
    }
    for (symbol_type i = static_cast<symbol_type>(token::epsilon) + 1; i < static_cast<symbol_type>(token::endmarker); i++) {
      callback(i);
    }
  }
  bool contain(symbol_type s) const override {
    return s < 256 || (s > static_cast<symbol_type>(token::epsilon) && s <  static_cast<symbol_type>( token::endmarker));
  }
  size_t size() const override {
	  return 256 + static_cast<symbol_type>(token::endmarker) -static_cast<symbol_type> (token::epsilon) - 1;
  }
  std::string name() const override { return "common_tokens"; }
};

} // namespace cyy::lang
