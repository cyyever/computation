/*
 * \file lr_item.cpp
 *
 * \author cyy
 * \date 2018-04-21
 */

#include "lr_item.hpp"

namespace cyy::computation {

  void LR_0_item_set ::add_kernel_item(const CFG &cfg, LR_0_item kernel_item) {

    std::vector<CFG::nonterminal_type> tmp_nonkernel_items;
    if (kernel_item.dot_pos < kernel_item.production.get_body().size()) {
      auto const &symbol =
          kernel_item.production.get_body()[kernel_item.dot_pos];
      if (auto ptr = symbol.get_nonterminal_ptr(); ptr) {
        if (nonkernel_items.count(*ptr) == 0) {
          tmp_nonkernel_items.push_back(*ptr);
        }
      }
    }

    kernel_items.emplace(std::move(kernel_item));
    while (!tmp_nonkernel_items.empty()) {
      auto nonkernel_item = std::move(tmp_nonkernel_items.back());
      tmp_nonkernel_items.pop_back();
      nonkernel_items.emplace(nonkernel_item);

      auto it = cfg.get_productions().find(nonkernel_item);
      for (auto const &body : it->second) {
        if (body[0].is_epsilon(cfg.get_alphabet())) {
          kernel_items.emplace(LR_0_item{{it->first, body}, 1});
          continue;
        }

        if (auto ptr = body[0].get_nonterminal_ptr(); ptr) {
          if (nonkernel_items.count(*ptr) == 0) {
            tmp_nonkernel_items.push_back(*ptr);
          }
        }
      }
    }
  }

  void
  LR_1_item_set::add_kernel_item(const CFG &cfg, const LR_0_item &kernel_item,
                                 std::set<CFG::terminal_type> lookahead_set) {
    auto const &body = kernel_item.production.get_body();
    if (kernel_item.dot_pos < body.size()) {

      add_nonkernel_item(cfg,
                         grammar_symbol_const_span_type(body).subspan(
                             static_cast<std::ptrdiff_t>(kernel_item.dot_pos)),
                         lookahead_set);
    }
    kernel_items[kernel_item].merge(lookahead_set);
  }

  void LR_1_item_set::add_nonkernel_item(
      const CFG &cfg, grammar_symbol_const_span_type view,
      const std::set<CFG::terminal_type> &lookahead_set) {

    if (view.empty()) {
      return;
    }

    auto ptr = view[0].get_nonterminal_ptr();

    if (!ptr) {
      return;
    }

    view = view.subspan(1);
    auto real_lookahead_set = cfg.first(view);

    if (real_lookahead_set.erase(cfg.get_alphabet().get_epsilon())) {
      real_lookahead_set.merge(std::set<CFG::terminal_type>(lookahead_set));
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
      if (new_body.size() == 1 && new_body[0].is_epsilon(cfg.get_alphabet())) {

        LR_0_item new_item{{*ptr, new_body}, 1};
        kernel_items[new_item] = nonkernel_items[*ptr];
        continue;
      }

      add_nonkernel_item(cfg, {new_body}, nonkernel_items[*ptr]);
    }
  }
} // namespace cyy::computation
