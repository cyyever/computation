/*!
 * \file slr_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "slr_grammar.hpp"
#include "dk.hpp"

namespace cyy::computation {

  std::pair<SLR_grammar::collection_type, SLR_grammar::goto_table_type>
  SLR_grammar::get_collection() const {
    DK_DFA dk(*this);
    collection_type collection;
    auto follow_sets = follow();
    for (auto const &[state, lr_0_item_set] :
         dk.get_LR_0_item_set_collection()) {
      LR_1_item_set set;
      for (auto const &item : lr_0_item_set.get_completed_items()) {
        set.add_kernel_item(*this, item, follow_sets[item.get_head()]);
      }
      collection[state] = std::move(set);
    }
    return {collection, dk.get_goto_table(true)};
  }
} // namespace cyy::computation
