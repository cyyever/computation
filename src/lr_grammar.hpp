/*!
 * \file lr_grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <functional>
#include <set>
#include <unordered_set>

#include "grammar.hpp"
#include "lang.hpp"
#include "lr_item.hpp"

namespace cyy::lang {

class LR_grammar : public CFG {

public:
  using CFG::CFG;
};
} // namespace cyy::lang
