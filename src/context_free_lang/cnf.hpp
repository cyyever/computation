/*!
 * \file cnf.hpp
 *
 */

#pragma once


#include "cfg.hpp"
#include "exception.hpp"
import std;

namespace cyy::computation {

  class CNF final : public CFG {
  public:
    CNF(const ALPHABET_ptr &alphabet_, nonterminal_type start_symbol_,
        production_set_type productions_)
        : CFG(alphabet_, start_symbol_, productions_) {
      if (!valid()) {
        throw exception::no_CNF("");
      }
    }

    explicit CNF(const CFG &cfg) : CFG(cfg) {
      if (!valid()) {
        throw exception::no_CNF("");
      }
    }
    explicit CNF(CFG &&cfg) : CFG(cfg) {
      if (!valid()) {
        throw exception::no_CNF("");
      }
    }

    CNF(const CNF &) = default;
    CNF &operator=(const CNF &) = default;

    CNF(CNF &&) noexcept = default;
    CNF &operator=(CNF &&) noexcept = default;

    ~CNF() override = default;

    [[nodiscard]] bool parse(symbol_string_view view) const;

  private:
    bool valid() const;
    void make_reverse_productions() const;

    mutable std::map<CFG_production::body_type, std::set<nonterminal_type>>
        reverse_productions;
  };
} // namespace cyy::computation
