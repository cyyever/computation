/*!
 * \file lalr_grammar.hpp
 *
 * \author cyy
 * \date 2018-09-24
 */

#pragma once

#include <functional>
#include <set>
#include "../lang/lang.hpp"
#include "canonical_lr_grammar.hpp"
#include "lr_item.hpp"

namespace cyy::computation {

class LALR_grammar : public canonical_LR_grammar {

public:
  LALR_grammar(const std::string &alphabet_name,
               const nonterminal_type &start_symbol_,
               std::map<nonterminal_type, std::vector<production_body_type>>
                   &productions_)
      : canonical_LR_grammar(alphabet_name, start_symbol_, productions_) { }

  std::pair<std::unordered_map<LR_1_item_set, uint64_t>,
            std::map<std::pair<uint64_t, grammar_symbol_type>, uint64_t>>
  canonical_collection();

private:
    //std::map<grammar_symbol_type,std::pair<bool,std::set<terminal_type>>> check_lookahead(const LR_0_item  & item,const grammar_symbol_type &symbol ) ;
    std::map<grammar_symbol_type,std::pair<bool,std::set<terminal_type>>> check_lookahead(const LR_0_item  & item) ;
  };
} // namespace cyy::computation
