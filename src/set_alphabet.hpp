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

#include "lang.hpp"

namespace cyy::lang {

class set_alphabet final : public ALPHABET {
public:
  set_alphabet(const std::set<symbol_type> &explicit_set_)
      : explicit_set(explicit_set_) {
    if (explicit_set.empty()) {
      throw std::invalid_argument("explicit set is empty");
    }
    auto it = std::max_element(explicit_set.begin(), explicit_set.end());
    max_symbol = *it;
  }

  symbol_type get_epsilon() const override { return max_symbol + 1; }
  symbol_type get_endmarker() const override { return max_symbol + 2; }

  void print(std::ostream &os, symbol_type symbol) const override {
    if (symbol == get_epsilon()) {
      os << "'epsilon'";
    } else if (symbol == get_endmarker()) {
      os << "$";
    } else {
      os << '\'' << static_cast<char>(symbol) << '\'';
    }
    return;
  }

  void foreach_symbol(
      const std::function<void(const symbol_type &)> &callback) const override {
    for (const auto &symbol : explicit_set) {
      callback(symbol);
    }
  }
  bool contain(symbol_type s) const override {
    return explicit_set.count(s) != 0;
  }
  size_t size() const override { return explicit_set.size(); }
  std::string name() const override { return alternative_name; }

private:
  symbol_type max_symbol;
  std::set<symbol_type> explicit_set;
};

} // namespace cyy::lang
