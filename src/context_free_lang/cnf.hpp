/*!
 * \file cnf.hpp
 *
 */

#pragma once

#include <functional>
#include <iostream>

#include "../exception.hpp"
#include "cfg.hpp"

namespace cyy::computation {

  class CNF final : public CFG {
  public:
    CNF(const std::string &alphabet_name, nonterminal_type start_symbol_,
        production_set_type productions_)
        : CFG(alphabet_name, start_symbol_, productions_) {
      if (!valid()) {
        throw exception::no_CNF("");
      }
    }

    CNF(const CFG &cfg) : CFG(cfg) {
      if (!valid()) {
        throw exception::no_CNF("");
      }
    }
    CNF(CFG &&cfg) : CFG(cfg) {
      if (!valid()) {
        throw exception::no_CNF("");
      }
    }

    CNF(const CNF &) = default;
    CNF &operator=(const CNF &) = default;

    CNF(CNF &&) noexcept = default;
    CNF &operator=(CNF &&) noexcept = default;

    virtual ~CNF() = default;

    [[nodiscard]] bool parse(symbol_string_view view) const;

  private:
    bool valid() const;
  };
} // namespace cyy::computation
