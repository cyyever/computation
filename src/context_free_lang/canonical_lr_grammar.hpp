/*!
 * \file canonical_lr_grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once
#include <optional>

#include "dk_1.hpp"
#include "lr_1_grammar.hpp"

namespace cyy::computation {

  class canonical_LR_grammar final : public LR_1_grammar {

  public:
    canonical_LR_grammar(ALPHABET_ptr alphabet_, nonterminal_type start_symbol_,
                         production_set_type productions_);

    std::pair<collection_type, goto_table_type> get_collection() const override;
    DPDA to_DPDA() const;
  };
} // namespace cyy::computation
