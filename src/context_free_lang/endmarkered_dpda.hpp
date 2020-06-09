/*!
 * \file endmarkered_dpda.hpp
 *
 */

#pragma once
#include <boost/dynamic_bitset.hpp>

#include "dpda.hpp"

namespace cyy::computation {

  class endmarkered_DPDA : public DPDA {
  public:
    explicit endmarkered_DPDA(DPDA dpda) : DPDA(std::move(dpda)) {
      to_endmarkered_DPDA();
    }

    void to_DPDA();

  private:
    state_set_type get_accept_states() const;
    void to_endmarkered_DPDA();
    bool transition_normalized{};
    void normalize_transitions();

    boost::dynamic_bitset<>
    state_set_to_bitset(const state_set_type &state_set);
  };

} // namespace cyy::computation
