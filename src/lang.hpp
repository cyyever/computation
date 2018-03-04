/*!
 * \file lang.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <functional>
#include <string>
#include <memory>
#include <string_view>

namespace cyy::lang {

using symbol_type = uint64_t;
class ALPHABET {
public:
  virtual ~ALPHABET() = default;

  virtual symbol_type get_epsilon() const = 0;

  virtual void foreach_symbol(
      const std::function<void(const symbol_type &)> &callback) const = 0;
  virtual bool contain(symbol_type s) const = 0;
  virtual size_t size() const = 0;
  virtual std::string name() const = 0;
};

std::unique_ptr<ALPHABET> make_alphabet(const std::string &name);

using symbol_string_view = std::basic_string_view<symbol_type>;
} // namespace cyy::lang
