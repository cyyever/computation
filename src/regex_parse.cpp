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
 * 	a -> a "" b | b
 * 	b -> b "*" | c
 * 	c -> (regex) | symbol
 * =>
 * 	regex -> ax
 * 	x -> "|" ax | epsilon
 * 	a -> b "" a | b
 * 	b -> cy
 * 	y -> "*" y | epsilon
 * 	c -> (regex) | symbol
 */
namespace {

std::shared_ptr<regex::syntax_node> nonterminal_regex(const ALPHABET &alphabet,
                                                      symbol_string_view &view);

std::shared_ptr<regex::syntax_node> nonterminal_c(const ALPHABET &alphabet,
                                                  symbol_string_view &view) {
  if (view.empty()) {
    return std::make_shared<regex::basic_node>(alphabet.get_epsilon());
  }

  switch (view[0]) {
  case '(': {
    view.remove_prefix(1);
    auto sub_node = nonterminal_regex(alphabet, view);
    if (view.empty() || view[0] != ')') {
      throw std::invalid_argument("lack ')'");
    }
    view.remove_prefix(1);
    return sub_node;
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

  auto c = view[0];
  view.remove_prefix(1);
  auto is_epsilon = (c == alphabet.get_epsilon());
  if (!is_epsilon && !alphabet.contain(c)) {
    throw std::invalid_argument(std::string("invalid symbol ") +
                                std::to_string(c));
  }
  if (is_epsilon) {
    return std::make_shared<regex::epsilon_node>();
  }
  return std::make_shared<regex::basic_node>(c);
}

std::shared_ptr<regex::syntax_node>
nonterminal_y(std::shared_ptr<regex::syntax_node> &&sub_node,
              symbol_string_view &view) {
  if (!view.empty() && view[0] == '*') {
    view.remove_prefix(1);
    return nonterminal_y(std::make_shared<regex::kleene_closure_node>(sub_node),
                         view);
  }
  return sub_node;
}

std::shared_ptr<regex::syntax_node> nonterminal_b(const ALPHABET &alphabet,
                                                  symbol_string_view &view) {
  return nonterminal_y(nonterminal_c(alphabet, view), view);
}

std::shared_ptr<regex::syntax_node> nonterminal_a(const ALPHABET &alphabet,
                                                  symbol_string_view &view) {
  auto left_node = nonterminal_b(alphabet, view);
  if (!view.empty() && view[0] != '|' && view[0] != '*') {
    return std::make_shared<regex::concat_node>(left_node,
                                                nonterminal_a(alphabet, view));
  }
  return left_node;
}

std::shared_ptr<regex::syntax_node>
nonterminal_x(std::shared_ptr<regex::syntax_node> &&left_node,
              const ALPHABET &alphabet, symbol_string_view &view) {
  if (!view.empty() && view[0] == '|') {
    view.remove_prefix(1);
    return std::make_shared<regex::union_node>(
        left_node,
        nonterminal_x(nonterminal_a(alphabet, view), alphabet, view));
  } else {
    return left_node;
  }
}

std::shared_ptr<regex::syntax_node>
nonterminal_regex(const ALPHABET &alphabet, symbol_string_view &view) {
  return nonterminal_x(nonterminal_a(alphabet, view), alphabet, view);
}
} // namespace
std::shared_ptr<regex::syntax_node>
regex::parse(symbol_string_view view) const {
  return nonterminal_regex(*alphabet, view);
}
} // namespace cyy::lang
