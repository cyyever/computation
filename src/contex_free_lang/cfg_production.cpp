/*!
 * \file grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "cfg_production.hpp"

namespace cyy::computation {

  bool CFG_production::is_epsilon(const ALPHABET &alphabet) const {
    return body.size() == 1 && body[0].is_epsilon(alphabet);
  }

  void CFG_production::print(std::ostream &os, const ALPHABET &alphabet) const {
    os << head << " -> ";
    for (const auto &grammal_symbol : body) {
      grammal_symbol.print(os, alphabet);
      os << ' ';
    }
    os << '\n';
  }

} // namespace cyy::computation
