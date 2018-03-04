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
    syntax_node(const std::shared_ptr<syntax_node> &left_node_)
        : type(TYPE::KLEENE_CLOSURE),left_node(left_node_) {
    }

    syntax_node(TYPE type_,
	const  std::shared_ptr<syntax_node> &left_node_,
	const  std::shared_ptr<syntax_node> &right_node_) 
        : type(type_),left_node(left_node_),right_node(right_node_) {
    }

    TYPE type;
    symbol_type symbol;
    std::shared_ptr<syntax_node> left_node, right_node;
  };

public:
  regex(const std::string &alphabet_name) {
    alphabet = make_alphabet(alphabet_name);
  }
  std::shared_ptr<syntax_node> parse(symbol_string_view view) const;

private:
  std::unique_ptr<ALPHABET> alphabet;
};
} // namespace cyy::lang
