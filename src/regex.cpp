/*!
 * \file regex.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "regex.hpp"

namespace cyy::lang {

/*
 * 	regex -> regex "|" a | a
 * 	a -> ab | b
 * 	b -> b "*" | c
 * 	c -> (regex) | symbol
 * =>
 * 	regex -> ax
 * 	x -> "|" ax | epsilon
 * 	a -> ba | b
 * 	b -> cy
 * 	y -> "*" y | epsilon
 * 	c -> (regex) | symbol
 */
namespace {

std::unique_ptr<regex::syntax_node> nonterminal_regex(const ALPHABET &alphabet,
                                                      symbol_string_view &view);

std::unique_ptr<regex::syntax_node> nonterminal_c(const ALPHABET &alphabet,
                                                  symbol_string_view &view) {
  if (view.empty()) {
    return std::make_unique<regex::syntax_node>(alphabet.get_epsilon());
  }

  switch (view[0]) {
  case '(': {
    view.remove_prefix(1);
    auto sub_expr = nonterminal_regex(alphabet, view);
    if (view.empty() || view[0] != ')') {
      throw std::invalid_argument("lack ')'");
    }
    view.remove_prefix(1);
    return sub_expr;
    break;
  }
  case '|':
  case '*':
    throw std::invalid_argument("lack symbol");
  case '\\':
    if (view.size() < 2) {
      throw std::invalid_argument("incomplete escape sequence");
    }
    view.remove_prefix(1);
    [[fallthrough]];
  default:
    break;
  }
  if (!alphabet.contain(view[0])) {
    throw std::invalid_argument(std::string("invalid symbol ") +
                                std::to_string(view[0]));
  }
  auto sub_expr = std::make_unique<regex::syntax_node>(view[0]);
  view.remove_prefix(1);
  return sub_expr;
}

std::unique_ptr<regex::syntax_node>
nonterminal_y(std::unique_ptr<regex::syntax_node> &sub_expr,
              symbol_string_view &view) {
  if (!view.empty() && view[0] == '*') {
    sub_expr = std::make_unique<regex::syntax_node>(sub_expr);
    view.remove_prefix(1);
    return nonterminal_y(sub_expr, view);
  }
  return std::move(sub_expr);
}

/*
std::unique_ptr<regex::syntax_node>
nonterminal_b(const ALPHABET &alphabet, symbol_string_view & view) {
        return nonterminal_y( nonterminal_c(alphabet,view) ,view);
}

std::unique_ptr<regex::syntax_node>
nonterminal_a(const ALPHABET &alphabet, symbol_string_view & view) {
  nonterminal_b(alphabet,view);
  if(!view.empty() && view[0]!='|' && view[0] !='*') {
    nonterminal_a(alphabet,view);
  }
}

std::unique_ptr<regex::syntax_node>
nonterminal_x(const ALPHABET &alphabet, symbol_string_view & view) {
  if(!view.empty() && view[0]=='|') {
    view.remove_prefix(1);
    nonterminal_a(alphabet,view);
    nonterminal_x(alphabet,view);
  }
}

std::unique_ptr<regex::syntax_node>
nonterminal_regex(const ALPHABET &alphabet, symbol_string_view & view) {
    nonterminal_a(alphabet,view);
    nonterminal_x(alphabet,view);
}
*/

} // namespace
std::unique_ptr<regex::syntax_node> regex::parse(symbol_string_view view) {

  return {};
  // return nonterminal_regex(this->alphabet,view);
}
} // namespace cyy::lang
