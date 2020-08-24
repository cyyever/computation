/*!
 * \file dcfg.hpp
 *
 */

#pragma once

#include <memory>

#include "dk.hpp"
#include "dpda.hpp"
#include "lr_0_grammar.hpp"

namespace cyy::computation {

  class DCFG : public LR_0_grammar {

  public:
    DCFG(ALPHABET_ptr alphabet_, nonterminal_type start_symbol_,
         production_set_type productions_);

    DPDA to_DPDA() const;
    const DK_DFA &get_dk() const { return *dk_dfa_ptr; }

  private:
    bool DK_test() const;
    std::pair<collection_type, goto_table_type> get_collection() const override;

  private:
    std::shared_ptr<DK_DFA> dk_dfa_ptr;
  };
} // namespace cyy::computation
