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
    DK_DFA const dk(*this);
    collection_type collection;
    auto follow_sets = follow();
    for (auto const &[state, lr_0_item_set] :
         dk.get_LR_0_item_set_collection()) {
      LR_1_item_set set;
      for (auto const &lr_0_item : lr_0_item_set.get_kernel_items()) {
        LR_1_item lr_1_item(lr_0_item);
        if (lr_0_item.completed()) {
          lr_1_item.add_lookahead_symbols(follow_sets[lr_0_item.get_head()]);
        }
        set.add_item(std::move(lr_1_item));
      }
      for (auto const &nonkernel_item : lr_0_item_set.get_nonkernel_items()) {
        set.add_nonkernel_item(nonkernel_item);
      }
      collection[state] = std::move(set);
    }
    return {collection, dk.get_goto_table(true)};
  }
} // namespace cyy::computation
