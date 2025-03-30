/*!
 * \file dcfg.hpp
 *
 */

#pragma once

#include "dk.hpp"
#include "dpda.hpp"
#include "lr_0_grammar.hpp"

namespace cyy::computation {

  class DCFG final : public LR_0_grammar {

  public:
    DCFG(ALPHABET_ptr alphabet_, nonterminal_type start_symbol_,
         production_set_type productions_);

    DPDA to_DPDA() const;
    const DK_DFA &get_dk() const { return *dk_dfa_opt; }

  private:
    bool DK_test() const;
    std::pair<collection_type, goto_table_type> get_collection() const override;

  private:
    std::optional<DK_DFA> dk_dfa_opt;
  };
} // namespace cyy::computation
