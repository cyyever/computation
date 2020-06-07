/*!
 * \file endmarkered_dpda.hpp
 *
 */

#pragma once

#include "dpda.hpp"

namespace cyy::computation {

  class endmarkered_DPDA : public DPDA {
  public:
    explicit endmarkered_DPDA(DPDA dpda) : DPDA(std::move(dpda)) { convert(); }

    /* DPDA to_DPDA() const ; */
  private:
    void convert();
    void normalize_transitions();
  };

} // namespace cyy::computation
