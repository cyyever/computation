/*!
 * \file lang.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>

#include "../exception.hpp"

namespace cyy::computation {

using symbol_type = char32_t;
class ALPHABET {
public:
  virtual ~ALPHABET() = default;

  symbol_type get_epsilon() const { return add_max_symbol(1); }
  symbol_type get_endmarker() const { return add_max_symbol(2); }
  symbol_type get_unincluded_symbol() const { return add_max_symbol(3); }
  bool is_epsilon(symbol_type s) const { return get_epsilon() == s; }

  virtual void foreach_symbol(
      const std::function<void(const symbol_type &)> &callback) const = 0;
  virtual bool contain(symbol_type s) const = 0;
  virtual size_t size() const = 0;
  void print(std::ostream &os, symbol_type symbol) const {
    if (symbol == get_epsilon()) {
      os << "'epsilon'";
    } else if (symbol == get_endmarker()) {
      os << "$";
    } else if (contain(symbol)) {
      print_symbol(os, symbol);
    } else {
      os << "(unkown symbol)";
    }
    return;
  }

  std::string get_name() const { return name; }

  static void regist(const std::string &name);
  static std::shared_ptr<ALPHABET> get(const std::string &name);

private:
  virtual void print_symbol(std::ostream &os, symbol_type symbol) const = 0;

  virtual symbol_type get_min_symbol() const = 0;
  virtual symbol_type get_max_symbol() const = 0;

  symbol_type add_max_symbol(size_t inc) const {
    const symbol_type max_symbol = get_max_symbol();
    const symbol_type new_symbol = max_symbol + static_cast<symbol_type>(inc);

    if (new_symbol < max_symbol) {
      throw cyy::computation::exception::symbol_overflow("");
    }
    return new_symbol;
  }

protected:
  std::string name;
};

using symbol_string = std::basic_string<symbol_type>;
using symbol_string_view = std::basic_string_view<symbol_type>;
using symbol_istringstream = std::basic_istringstream<symbol_type>;
using symbol_istream = std::basic_istream<symbol_type>;

} // namespace cyy::computation
