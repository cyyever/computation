/*!
 * \file grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <variant>
#include <string>
#include <string_view>

#include "../lang/lang.hpp"

namespace cyy::computation{
  class  grammar_symbol_type {

  public:
    using terminal_type = symbol_type;
    using nonterminal_type = std::string;

    grammar_symbol_type()=default;
    grammar_symbol_type(nonterminal_type  nonterminal):symbol(std::move(nonterminal)){}
    grammar_symbol_type(terminal_type  terminal):symbol(terminal) {}

    constexpr bool operator==(const grammar_symbol_type & rhs) const  {
 return symbol == rhs.symbol;     
    }

    constexpr bool operator<(const grammar_symbol_type & rhs) const  {
 return symbol < rhs.symbol;     
    }

    constexpr bool operator!=(const grammar_symbol_type & rhs) const  {
      return !operator==(rhs); 
    }

    bool is_terminal() const { return std::holds_alternative<terminal_type>(symbol); }
    bool is_nonterminal() const { return std::holds_alternative<nonterminal_type>(symbol); }

    auto get_terminal_ptr()const  ->auto const {
       return std::get_if<terminal_type>(&symbol);
    }

    auto get_nonterminal_ptr()const  ->auto const {
       return std::get_if<nonterminal_type>(&symbol);
    }

  void print(std::ostream &os,const std::string &alphabet_name) const {
    if(is_terminal()) {
      ALPHABET::get(alphabet_name)->print(os,*get_terminal_ptr());
    } else {
      os<<*get_nonterminal_ptr();
    }
  }

  private:
    std::variant<terminal_type, nonterminal_type> symbol;
};

  using grammar_symbol_string =
      std::basic_string<grammar_symbol_type>;
  using grammar_symbol_string_view =
      std::basic_string_view<grammar_symbol_type>;
}
