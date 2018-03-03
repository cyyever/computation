/*!
 * \file regex.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "regex.hpp"


namespace cyy::compiler {

/*
 * 	regex -> regex "|" a | a
 * 	a -> ab | b
 * 	b -> b* | c
 * 	c -> (regex) | symbol
 * =>
 * 	regex -> ax
 * 	x -> "|" ax | epsilon 
 * 	a -> ba | b
 * 	b -> cy
 * 	y -> *y | epsilon
 * 	c -> (regex) | symbol
 */
  std::unique_ptr<regex> regex::parse_string(symbol_type *str,size_t str_len) {
    if(str_len==0) {
      return std::make_unique<regex>(epsilon);
    }
    if(*str=='(') {
 //     auto sub_regex=parse_string(

    }

    return {};
  }
}
