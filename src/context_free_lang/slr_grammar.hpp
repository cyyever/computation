/*!
 * \file slr_grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <unordered_map>

#include "../hash.hpp"
#include "lr_grammar.hpp"

namespace cyy::computation {

  class SLR_grammar final : public LR_grammar {

  public:
    using LR_grammar::LR_grammar;

  private:
    void construct_parsing_table() const override;
  };
} // namespace cyy::computation
