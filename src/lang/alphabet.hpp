/*!
 * \file alphabet.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>

#include "../exception.hpp"
#include "symbol.hpp"

namespace cyy::computation {

  class ALPHABET {

  public:
    class iterator {
    public:
      iterator(const ALPHABET *ptr_, size_t index_)
          : ptr(ptr_), index(index_) {}
      iterator operator++() {
        ++index;
        return *this;
      }
      bool operator!=(const iterator &other) const {
        return ptr != other.ptr || index != other.index;
      }
      symbol_type operator*() const { return ptr->get_symbol(index); }

    private:
      const ALPHABET *ptr;
      size_t index;
    };

  public:
    virtual ~ALPHABET() = default;

    symbol_type get_endmarker() const { return add_max_symbol(2); }
    symbol_type get_unincluded_symbol() const { return add_max_symbol(3); }

    iterator begin() const { return iterator(this, 0); }
    iterator end() const { return iterator(this, size()); }

    virtual bool contain(symbol_type s) const = 0;
    virtual size_t size() const = 0;
    void print(std::ostream &os, symbol_type symbol) const;

    std::string get_name() const { return name; }

    bool operator==(const ALPHABET &rhs) const {
      return (this == &rhs) || this->name == rhs.name;
    }

    bool operator!=(const ALPHABET &rhs) const { return !operator==(rhs); }
    virtual bool contains_ASCII() const { return false; }

    static void regist(const std::string &name);
    static std::shared_ptr<ALPHABET> get(std::string_view name);
    static void set(std::shared_ptr<ALPHABET> alphabet);

  private:
    virtual void print_symbol(std::ostream &os, symbol_type symbol) const = 0;

    virtual symbol_type get_min_symbol() const = 0;
    virtual symbol_type get_max_symbol() const = 0;
    virtual symbol_type get_symbol(size_t index) const = 0;

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

  private:
    static inline std::map<std::string, std::shared_ptr<ALPHABET>> factory;
  };

  void print_symbol_string(std::ostream &os, const symbol_string &str,
                           const ALPHABET &alphabet);
} // namespace cyy::computation
