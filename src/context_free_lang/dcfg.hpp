/*!
 * \file dcfg.hpp
 *
 */

#pragma once

#include <memory>

#include "dk.hpp"
#include "dpda.hpp"
#include "lr_grammar.hpp"

namespace cyy::computation {

  class DCFG : public LR_grammar {

  public:
    DCFG(std::shared_ptr<ALPHABET> alphabet_, nonterminal_type start_symbol_,
         production_set_type productions_);

    DPDA to_DPDA() const;

  private:
    bool DK_test() const;
    void construct_parsing_table() const override;

  private:
    std::shared_ptr<DK_DFA> dk_dfa_ptr;
  };
} // namespace cyy::computation
