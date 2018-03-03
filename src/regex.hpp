/*!
 * \file regex.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <memory>

namespace cyy::compiler {

class regex {
  public:
  using symbol_type = uint64_t;
  static constexpr symbol_type epsilon = 0;
  enum class regex_type {
    BASIC,
    UNION,
    CONCAT,
    KLEENE_CLOSURE
  };

  public:
  regex(symbol_type symbol_):type(regex_type::BASIC),symbol(symbol_) { }
  regex(regex_type type_,std::unique_ptr<regex> & sub_regex1,std::unique_ptr<regex> & sub_regex2):type(type_) {
    std::swap(sub_regexes[0],sub_regex1);
    std::swap(sub_regexes[1],sub_regex2);
  }

  static std::unique_ptr<regex> parse_string(symbol_type *str,size_t str_len);

  private:
  regex_type type;
  symbol_type symbol;
  std::unique_ptr<regex> sub_regexes[2];

};
}
