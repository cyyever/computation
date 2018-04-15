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

namespace cyy::lang {

using symbol_type = char32_t;
class ALPHABET {
public:
  virtual ~ALPHABET() = default;

  virtual symbol_type get_epsilon() const = 0;
  virtual symbol_type get_endmarker() const = 0;

  virtual void foreach_symbol(
      const std::function<void(const symbol_type &)> &callback) const = 0;
  virtual bool contain(symbol_type s) const = 0;
  bool is_epsilon(symbol_type s) const { return get_epsilon() == s; }

  virtual size_t size() const = 0;
  virtual void print(std::ostream &os, symbol_type symbol) const = 0;

  virtual std::string name() const = 0;
  static void regist(const std::string &name);
  static std::shared_ptr<ALPHABET> get(const std::string &name);

protected:
  std::string alternative_name;

private:
  static std::map<std::string, std::shared_ptr<ALPHABET>> factory;
};

using symbol_string = std::basic_string<symbol_type>;
using symbol_string_view = std::basic_string_view<symbol_type>;
using symbol_istringstream = std::basic_istringstream<symbol_type>;

} // namespace cyy::lang
