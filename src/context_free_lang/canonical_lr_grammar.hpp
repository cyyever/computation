/*!
 * \file canonical_lr_grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include "dpda.hpp"
#include "lr_1_grammar.hpp"

namespace cyy::computation {

  class canonical_LR_grammar : public LR_1_grammar {

  public:
    using LR_1_grammar::LR_1_grammar;

    std::pair<collection_type, goto_table_type> get_collection() const override;
    DPDA to_DPDA() const;
  };
} // namespace cyy::computation
