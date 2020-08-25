/*!
 * \file canonical_lr_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "canonical_lr_grammar.hpp"
#include "../exception.hpp"

namespace cyy::computation {
  canonical_LR_grammar::canonical_LR_grammar(ALPHABET_ptr alphabet_,
                                             nonterminal_type start_symbol_,
                                             production_set_type productions_)

      : LR_1_grammar(alphabet_, start_symbol_, std::move(productions_)) {}

  std::pair<canonical_LR_grammar::collection_type,
            canonical_LR_grammar::goto_table_type>
  canonical_LR_grammar::get_collection() const {
    DK_1_DFA dk_1_dfa(*this);
    auto const &collection = dk_1_dfa.get_LR_1_item_set_collection();
    auto goto_table = dk_1_dfa.get_goto_table();
    return {collection, goto_table};
  }

} // namespace cyy::computation
