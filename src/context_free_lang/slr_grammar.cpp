/*!
 * \file slr_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "slr_grammar.hpp"

namespace cyy::computation {

  std::pair<SLR_grammar::collection_type, SLR_grammar::goto_table_type>
  SLR_grammar::get_collection() const {

    auto const &[lr_0_item_set_collection, goto_table] =
        get_lr_0_item_set_collection();

    collection_type collection;
    auto follow_sets = follow();
    for (auto const &[state, lr_0_item_set] : lr_0_item_set_collection) {
      LR_1_item_set set;
      for (auto const &item : lr_0_item_set.get_kernel_items()) {
        if (item.completed()) {
          set.add_kernel_item(*this, item, follow_sets[item.get_head()]);
        }
      }
      collection[state] = std::move(set);
    }
    return {collection, goto_table};
  }
} // namespace cyy::computation
