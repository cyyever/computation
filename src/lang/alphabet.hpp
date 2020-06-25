/*!
 * \file alphabet.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../exception.hpp"
#include "symbol.hpp"

namespace cyy::computation {

  class ALPHABET {

  public:
    class iterator final {
    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = symbol_type;
      using difference_type = ptrdiff_t;
      using pointer = value_type *;
      using reference = const value_type &;
      iterator(const ALPHABET *ptr_, size_t index_)
          : ptr(ptr_), index(index_) {}
      iterator(const iterator &) = default;
      iterator &operator=(const iterator &) = default;

      iterator(iterator &&) noexcept = default;
      iterator &operator=(iterator &&) noexcept = default;
      ~iterator() = default;

      bool operator==(const iterator &other) const = default;
      iterator &operator++() {
        ++index;
        return *this;
      }
      iterator operator++(int) {
        index++;
        return *this;
      }
      iterator &operator+=(difference_type rhs) {
        index = static_cast<size_t>(static_cast<difference_type>(index) + rhs);
        return *this;
      }
      iterator operator+(difference_type rhs) {
        iterator new_it(*this);
        new_it += rhs;
        return new_it;
      }
      value_type operator*() const { return ptr->get_symbol(index); }

    private:
      const ALPHABET *ptr;
      size_t index;
    };

  public:
    explicit ALPHABET(std::string_view name_) { set_name(name_); }
    ALPHABET(const ALPHABET &) = default;
    ALPHABET &operator=(const ALPHABET &) = default;

    ALPHABET(ALPHABET &&) noexcept = default;
    ALPHABET &operator=(ALPHABET &&) noexcept = default;
    virtual ~ALPHABET() = default;

    symbol_type get_unincluded_symbol() const { return add_max_symbol(2); }

    auto get_view(bool include_endmark = false) const {
      auto alphabet_size = size();
      auto bound = alphabet_size;
      if (include_endmark) {
        bound++;
      }
      return std::views::iota(static_cast<size_t>(0), bound) |
             std::views::transform([alphabet_size, this](auto idx) {
               if (idx == alphabet_size) {
                 return endmarker;
               }
               return get_symbol(idx);
             });
    }

    iterator begin() const noexcept { return iterator(this, 0); }
    iterator end() const noexcept { return iterator(this, size()); }

    symbol_type get_min_symbol() const { return get_symbol(0); }
    symbol_type get_max_symbol() const {
      if (contain(endmarker)) {
        return get_symbol(size() - 2);
      }
      return get_symbol(size() - 1);
    }

    virtual bool contain(symbol_type s) const = 0;
    virtual size_t size() const = 0;
    std::string to_string(symbol_type symbol) const {
      if (symbol == endmarker) {
        return "$";
      }
      if (contain(symbol)) {
        return __to_string(symbol);
      }

      return "(unknown symbol)";
    }

    std::string get_name() const { return name; }

    bool operator==(const ALPHABET &rhs) const = default;
    virtual bool contains_ASCII() const { return false; }

    static std::shared_ptr<ALPHABET> get(std::string_view name,
                                         bool endmarkered = false);
    static void set(const std::shared_ptr<ALPHABET> &alphabet);
    static constexpr symbol_type endmarker =
        std::numeric_limits<symbol_type>::max() - 1;

  protected:
    void set_name(std::string_view name_) {
      if (name_.empty()) {
        throw cyy::computation::exception::empty_alphabet_name("");
      }
      name = std::move(name_);
    }
    virtual std::string __to_string(symbol_type symbol) const {
      return {'\'', static_cast<char>(symbol), '\''};
    }

  private:
    virtual symbol_type get_symbol(size_t index) const = 0;

    symbol_type add_max_symbol(size_t inc) const {
      const symbol_type max_symbol = get_max_symbol();
      const symbol_type new_symbol = max_symbol + static_cast<symbol_type>(inc);

      if (new_symbol <= max_symbol) {
        throw cyy::computation::exception::symbol_overflow("");
      }
      return new_symbol;
    }
    static void register_factory();

  private:
    std::string name;

  private:
    static inline std::unordered_map<std::string, std::shared_ptr<ALPHABET>>
        factory;
  };

  inline auto endmarkered_symbol_string(symbol_string_view str) {
    auto size = str.size() + 1;
    return std::views::iota(static_cast<size_t>(0), size) |
           std::views::transform([str, size](auto idx) {
             if (idx + 1 == size) {
               return ALPHABET::endmarker;
             }
             return str[idx];
           });
  }

  void print_symbol_string(std::ostream &os, const symbol_string &str,
                           const ALPHABET &alphabet);
} // namespace cyy::computation
