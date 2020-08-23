/*
 * \file lr_item.cpp
 *
 * \author cyy
 * \date 2018-04-21
 */

#include "lr_1_item.hpp"

namespace cyy::computation {
  void LR_1_item::add_lookahead_symbol(CFG::terminal_type lookahead_symbol) {
    lookahead_symbols.insert(lookahead_symbol);
  }

  void LR_1_item_set::add_kernel_item(const CFG &cfg,
                                      const LR_0_item &kernel_item,
                                      CFG::terminal_set_type lookahead_set) {
    if (!kernel_item.completed()) {
      add_nonkernel_item(cfg, kernel_item.prefix(), lookahead_set);
    }
    kernel_items[kernel_item].merge(std::move(lookahead_set));
  }

  std::unordered_map<grammar_symbol_type, LR_1_item_set>
  LR_1_item_set::go(const CFG &cfg) const {

    std::unordered_map<grammar_symbol_type, LR_1_item_set> res;

    for (auto const &[kernel_item, lookahead_set] : get_kernel_items()) {
      if (kernel_item.completed()) {
        continue;
      }
      auto const &symbol = kernel_item.get_grammar_symbal();
      auto new_kernel_item = kernel_item;
      new_kernel_item.go();
      res[symbol].add_kernel_item(cfg, new_kernel_item, lookahead_set);
    }

    for (auto const &[nonterminal, lookahead_set] : get_nonkernel_items()) {
      for (auto const &body : cfg.get_bodies(nonterminal)) {
        if (body.empty()) {
          continue;
        }
        LR_0_item new_kernel_item{{nonterminal, body}, 1};
        res[body[0]].add_kernel_item(cfg, new_kernel_item, lookahead_set);
      }
    }
    return res;
  }

  void LR_1_item_set::add_nonkernel_item(
      const CFG &cfg, grammar_symbol_const_span_type view,
      const CFG::terminal_set_type &lookahead_set) {

    assert(!view.empty());
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

    for (auto const &new_body : cfg.get_bodies(*ptr)) {
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
