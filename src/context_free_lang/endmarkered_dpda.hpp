/*!
 * \file endmarkered_dpda.hpp
 *
 */

#pragma once

#include "dpda.hpp"

namespace cyy::computation {

  class endmarkered_DPDA : public DPDA {
  public:
    explicit endmarkered_DPDA(DPDA dpda);

    DPDA to_DPDA() const;

  private:
    state_set_type get_accept_states() const;
    void normalize_transitions();

  private:
    bool transition_normalized{};
  };

} // namespace cyy::computation
