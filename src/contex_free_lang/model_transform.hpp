/*!
 * \file model_transform.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include "../regular_lang/nfa.hpp"
#include "cfg.hpp"

namespace cyy::computation {

  CFG NFA_to_CFG(const NFA &nfa);

} // namespace cyy::computation
