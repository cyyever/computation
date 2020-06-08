/*!
 * \file endmarkered_dpda.hpp
 *
 */

#pragma once

#include "dpda.hpp"

namespace cyy::computation {

  class endmarkered_DPDA : public DPDA {
  public:
    explicit endmarkered_DPDA(DPDA dpda) : DPDA(std::move(dpda)) { to_endmarkered_DPDA(); }

    DPDA to_DPDA() ;
  private:
  state_set_type get_accept_states() const;
    void to_endmarkered_DPDA();
    void normalize_transitions();
  };

} // namespace cyy::computation
