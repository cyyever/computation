/*!
 * \file slr_grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include "lr_1_grammar.hpp"

namespace cyy::computation {

  class SLR_grammar final : public LR_1_grammar {
  public:
    using LR_1_grammar::LR_1_grammar;

    std::pair<collection_type, goto_table_type> get_collection() const override;
  };
} // namespace cyy::computation
