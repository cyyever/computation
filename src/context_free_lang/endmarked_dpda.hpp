/*!
 * \file endmarked_dpda.hpp
 *
 */

#pragma once

#include "dpda.hpp"

namespace cyy::computation {

  class endmarked_DPDA : public DPDA {
  public:
    explicit endmarked_DPDA(DPDA dpda);

    DPDA to_DPDA() const;

    void prepare_CFG_conversion();

  private:
    state_set_type get_accept_states() const;
    void normalize_transitions();

  private:
    bool transition_normalized{};
  };

} // namespace cyy::computation
