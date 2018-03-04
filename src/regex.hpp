/*!
 * \file regex.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include "lang.hpp"

namespace cyy::lang {

class regex {

public:
  struct syntax_node {
    enum struct TYPE { BASIC, UNION, CONCAT, KLEENE_CLOSURE };

    syntax_node(symbol_type symbol_) : type(TYPE::BASIC), symbol(symbol_) {}
    syntax_node(std::unique_ptr<syntax_node> &left_)
        : type(TYPE::KLEENE_CLOSURE) {
      left.swap(left_);
    }

    syntax_node(TYPE type_, std::unique_ptr<syntax_node> &left_,
                std::unique_ptr<syntax_node> &right_)
        : type(type_) {
      left.swap(left_);
      right.swap(right_);
    }

    TYPE type;
    symbol_type symbol;
    std::unique_ptr<syntax_node> left, right;
  };

public:
  regex(const std::string &alphabet_name) {
    alphabet = make_alphabet(alphabet_name);
  }
  std::unique_ptr<syntax_node> parse(symbol_string_view view);

private:
  std::unique_ptr<ALPHABET> alphabet;
};
} // namespace cyy::lang
