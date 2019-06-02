/*!
 * \file set_alphabet.hpp
 *
 * \brief
 * \author cyy
 * \date 2018-03-31
 */
#pragma once

#include <algorithm>
#include <set>

#include "../exception.hpp"
#include "alphabet.hpp"

namespace cyy::computation {

  class set_alphabet : public ALPHABET {
  public:
    explicit set_alphabet(std::set<symbol_type> explicit_set_,
                          std::string_view name_)
        : explicit_set(std::move(explicit_set_)) {
      if (explicit_set.empty()) {
        throw exception::empty_alphabet("explicit set is empty");
      }
      name = name_;
    }

    void foreach_symbol(const std::function<void(const symbol_type &)>
                            &callback) const override {
      for (const auto &symbol : explicit_set) {
        callback(symbol);
      }
    }
    bool contain(symbol_type s) const noexcept override {
      return explicit_set.count(s) != 0;
    }
    size_t size() const noexcept override { return explicit_set.size(); }

    std::set<symbol_type> get_symbols() const override { return explicit_set; }

  private:
    void print_symbol(std::ostream &os, symbol_type symbol) const override {
      os << '\'' << static_cast<char>(symbol) << '\'';
      return;
    }

    symbol_type get_max_symbol() const noexcept override {
      return *explicit_set.crbegin();
    }
    symbol_type get_min_symbol() const noexcept override {
      return *explicit_set.rbegin();
    }

  private:
    std::set<symbol_type> explicit_set;
  };

} // namespace cyy::computation
