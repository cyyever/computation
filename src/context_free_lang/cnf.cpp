/*!
 * \file cnf.cpp
 *
 * \brief
 */

#include <cassert>

#include "cnf.hpp"

namespace cyy::computation {
  bool CNF::valid() const {
    for (auto &[head, bodies] : productions) {
      for (auto &body : bodies) {
        auto body_size = body.size();
        if (body_size > 2) {
          return false;
        }
        if (body_size == 0) {
          if (head != start_symbol) {
            return false;
          }
        }
        if (body_size == 1) {
          if (!body[0].is_terminal()) {
            return false;
          }
        }
        if (body_size == 2) {
          if (std::any_of(body.begin(), body.end(), [*this](auto const &g) {
                return g.is_terminal() || g == start_symbol;
              })) {
            return false;
          }
        }
      }
    }
    return true;
  }

} // namespace cyy::computation
