/*!
 * \file turing_machine.hpp
 *
 */

#pragma once

#include "multi_tape_turing_machine_base.hpp"

namespace cyy::computation {
  class single_tape_Turing_machine : public multi_tape_Turing_machine_base<1> {
  public:
    using multi_tape_Turing_machine_base::multi_tape_Turing_machine_base;
  };
} // namespace cyy::computation
