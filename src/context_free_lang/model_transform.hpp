/*!
 * \file model_transform.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include "cfg.hpp"
#include "dcfg.hpp"
#include "endmarked_dpda.hpp"
#include "pda.hpp"
#include "regular_lang/nfa.hpp"

namespace cyy::computation {
  CFG NFA_to_CFG(const NFA &nfa);
  PDA CFG_to_PDA(CFG cfg);
  CFG PDA_to_CFG(PDA pda);
  DCFG DPDA_to_DCFG(endmarked_DPDA dpda);
} // namespace cyy::computation
