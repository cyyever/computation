/*!
 * \file grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "cfg_production.hpp"

namespace cyy::computation {

  bool CFG_production::is_epsilon() const { return body.empty(); }

  void CFG_production::print(std::ostream &os, const ALPHABET &alphabet) const {
    os << head << " -> ";
    print_body(os, alphabet);
    os << '\n';
  }
  void CFG_production::print_body(std::ostream &os,
                                  const ALPHABET &alphabet) const {
    if (body.empty()) {
      os << "'epsilon'";
    } else {
      for (const auto &grammal_symbol : body) {
        grammal_symbol.print(os, alphabet);
        os << ' ';
      }
    }
  }
} // namespace cyy::computation
