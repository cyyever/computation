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

#include "../exception.hpp"
#include "alphabet.hpp"

namespace cyy::computation {

  class set_alphabet : public ALPHABET {
  public:
    explicit set_alphabet(const std::set<symbol_type> &explicit_set_,
                          std::string_view name_)
        : explicit_set(explicit_set_.begin(), explicit_set_.end()) {
      if (explicit_set.empty()) {
        throw exception::empty_alphabet("explicit set is empty");
      }
      name = name_;
    }

    bool contain(symbol_type s) const noexcept override {
      return ranges::v3::binary_search(explicit_set, s);
    }
    size_t size() const noexcept override { return explicit_set.size(); }

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

    symbol_type get_symbol(size_t index) const noexcept override {
      auto it = explicit_set.begin();
      std::advance(it, index);
      return *it;
    }

  private:
    std::vector<symbol_type> explicit_set;
  };

} // namespace cyy::computation
