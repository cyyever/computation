/*!
 * \file model_transform.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include "../regular_lang/nfa.hpp"
#include "cfg.hpp"
#include "pda.hpp"

namespace cyy::computation {

  CFG NFA_to_CFG(const NFA &nfa);

  PDA CFG_to_PDA(CFG cfg);
} // namespace cyy::computation
