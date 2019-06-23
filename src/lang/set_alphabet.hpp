/*!
 * \file set_alphabet.hpp
 *
 * \brief
 * \author cyy
 * \date 2018-03-31
 */
#pragma once

#include <algorithm>
#include <range/v3/algorithm.hpp>
#include <set>
#include <vector>

#include "../exception.hpp"
#include "alphabet.hpp"

namespace cyy::computation {

  class set_alphabet : public ALPHABET {
  public:
    explicit set_alphabet(const std::set<symbol_type> &symbol_set,
                          std::string_view name_)
        : symbols(symbol_set.begin(), symbol_set.end()) {
      if (symbols.empty()) {
        throw exception::empty_alphabet("symbol set is empty");
      }
      name = name_;
    }

    bool contain(symbol_type s) const noexcept override {
      return ranges::v3::binary_search(symbols, s);
    }
    size_t size() const noexcept override { return symbols.size(); }

  private:
    void print_symbol(std::ostream &os, symbol_type symbol) const override {
      os << '\'' << static_cast<char>(symbol) << '\'';
      return;
    }

    symbol_type get_max_symbol() const noexcept override {
      return *symbols.crbegin();
    }
    symbol_type get_min_symbol() const noexcept override {
      return *symbols.rbegin();
    }

    symbol_type get_symbol(size_t index) const noexcept override {
      return symbols[index];
    }

  private:
    std::vector<symbol_type> symbols;
  };

} // namespace cyy::computation
