/*!
 * \file lalr_grammar.hpp
 *
 * \author cyy
 * \date 2018-09-24
 */

#pragma once

/* #include <unordered_map> */

#include "canonical_lr_grammar.hpp"

namespace cyy::computation {

  class LALR_grammar final : public canonical_LR_grammar {

  public:
    using canonical_LR_grammar::canonical_LR_grammar;

    std::pair<collection_type, goto_table_type> get_collection() const override;
  };
} // namespace cyy::computation
