/*!
 * \file alphabet.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

#include "../exception.hpp"
#include "symbol.hpp"

namespace cyy::computation {

  class ALPHABET {
  public:
    virtual ~ALPHABET() = default;

    //   symbol_type get_epsilon() const { return add_max_symbol(1); }
    symbol_type get_endmarker() const { return add_max_symbol(2); }
    symbol_type get_unincluded_symbol() const { return add_max_symbol(3); }

    virtual void foreach_symbol(
        const std::function<void(const symbol_type &)> &callback) const = 0;
    virtual bool contain(symbol_type s) const = 0;
    virtual size_t size() const = 0;
    void print(std::ostream &os, symbol_type symbol) const;

    std::string get_name() const { return name; }

    bool operator==(const ALPHABET &rhs) const {
      return (this == &rhs) || this->name == rhs.name;
    }

    bool operator!=(const ALPHABET &rhs) const { return !operator==(rhs); }
    static void regist(const std::string &name);
    static std::shared_ptr<ALPHABET> get(std::string_view name);

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

  void print_symbol_string(std::ostream &os, const symbol_string &str,
                           const ALPHABET &alphabet);
} // namespace cyy::computation
