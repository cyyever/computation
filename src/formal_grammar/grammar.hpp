/*!
 * \file grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "../lang/lang.hpp"

namespace cyy::computation {
class grammar_symbol_type : public std::variant<symbol_type, std::string> {

public:
  using terminal_type = symbol_type;
  using nonterminal_type = std::string;
  using std::variant<symbol_type, std::string>::variant;

  bool is_terminal() const noexcept {
    return std::holds_alternative<terminal_type>(*this);
  }
  bool is_nonterminal() const noexcept {
    return std::holds_alternative<nonterminal_type>(*this);
  }

  const terminal_type *get_terminal_ptr() const noexcept {
    return std::get_if<terminal_type>(this);
  }

  const nonterminal_type *get_nonterminal_ptr() const noexcept {
    return std::get_if<nonterminal_type>(this);
  }

  void print(std::ostream &os, const std::string &alphabet_name) const {
    if (is_terminal()) {
      ALPHABET::get(alphabet_name)->print(os, *get_terminal_ptr());
    } else {
      os << *get_nonterminal_ptr();
    }
  }
};

using grammar_symbol_string = std::basic_string<grammar_symbol_type>;
using grammar_symbol_string_view = std::basic_string_view<grammar_symbol_type>;
} // namespace cyy::computation
