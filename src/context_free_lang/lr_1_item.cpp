/*
 * \file lr_item.cpp
 *
 * \author cyy
 * \date 2018-04-21
 */

#include "lr_1_item.hpp"

namespace cyy::computation {
  void
  LR_1_item_set::add_kernel_item(const CFG &cfg, const LR_0_item &kernel_item,
                                 std::set<CFG::terminal_type> lookahead_set) {
    if (!kernel_item.completed()) {
      add_nonkernel_item(cfg, kernel_item.prefix(), lookahead_set);
    }
    kernel_items[kernel_item].merge(std::move(lookahead_set));
  }

  void LR_1_item_set::add_nonkernel_item(
      const CFG &cfg, grammar_symbol_const_span_type view,
      const std::set<CFG::terminal_type> &lookahead_set) {

    assert(!views.empty());
    if (view.empty()) {
      throw std::runtime_error("invalid LR_0_item");
    }

    auto ptr = view[0].get_nonterminal_ptr();

    if (!ptr) {
      return;
    }

    view = view.subspan(1);
    auto [real_lookahead_set, epsilon_in_first] = cfg.first(view);

    if (epsilon_in_first) {
      real_lookahead_set.insert(lookahead_set.begin(), lookahead_set.end());
    }

    decltype(real_lookahead_set) diff;

    std::set_difference(real_lookahead_set.begin(), real_lookahead_set.end(),
                        nonkernel_items[*ptr].begin(),
                        nonkernel_items[*ptr].end(),
                        std::inserter(diff, diff.begin()));

    if (diff.empty()) {
      return;
    }

    nonkernel_items[*ptr].merge(diff);

    auto it = cfg.get_productions().find(*ptr);

    for (auto const &new_body : it->second) {
      if (new_body.empty()) {
        LR_0_item new_item{{*ptr, new_body}, 0};
        kernel_items[new_item] = nonkernel_items[*ptr];
        continue;
      }

      add_nonkernel_item(cfg, {new_body}, nonkernel_items[*ptr]);
    }
  }
  std::string LR_1_item_set::MMA_draw(const ALPHABET &alphabet) const {
    return "";
  }

} // namespace cyy::computation
