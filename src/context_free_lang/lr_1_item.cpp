/*
 * \file lr_item.cpp
 *
 * \author cyy
 * \date 2018-04-21
 */

#include "lr_1_item.hpp"

#include <cstddef>

namespace cyy::computation {
  CFG::terminal_set_type LR_1_item::follow_of_dot(const CFG &cfg) const {
    assert(!completed());
    auto [first_set, epsilon_in_first] =
        cfg.first(grammar_symbol_const_span_type(
            get_body().begin() + static_cast<ptrdiff_t>(get_dot_pos()) + 1,
            get_body().end()

                ));
    if (epsilon_in_first) {
      first_set.merge(CFG::terminal_set_type(lookahead_symbols));
    }
    return first_set;
  }
  void LR_1_item::add_lookahead_symbol(CFG::terminal_type lookahead_symbol) {
    lookahead_symbols.insert(lookahead_symbol);
  }
  void
  LR_1_item::add_lookahead_symbols(CFG::terminal_set_type lookahead_symbols_) {
    lookahead_symbols.merge(lookahead_symbols_);
  }

  std::string LR_1_item::MMA_draw(const CFG &cfg) const {
    auto cmd = std::string("Labeled[") + LR_0_item::MMA_draw(cfg);
    cmd += ",{";
    for (auto a : lookahead_symbols) {
      cmd += cfg.get_alphabet().MMA_draw(a);
      cmd.push_back(',');
    }
    cmd.back() = '}';
    cmd += ",Right]";
    return cmd;
  }

  new_LR_1_item_set::new_LR_1_item_set(const LR_0_item_set &set) {
    for (auto lr_0_item : set.get_kernel_items()) {
      kernel_items.emplace(std::move(lr_0_item));
    }

    for (auto nonkernel_item : set.get_nonkernel_items()) {
      nonkernel_items.try_emplace(std::move(nonkernel_item),
                                  CFG::terminal_set_type{});
    }
  }

  std::string new_LR_1_item_set::MMA_draw(const CFG &cfg) const {
    std::string cmd = "Framed[TableForm[{";
    for (auto const &item : kernel_items) {
      cmd += item.MMA_draw(cfg);
      cmd.push_back(',');
    }
    if (!nonkernel_items.empty()) {
      cmd += "Framed[TableForm[{";
      for (auto const &item : expand_nonkernel_items(cfg)) {
        cmd += item.MMA_draw(cfg);
        cmd.push_back(',');
      }
      cmd.pop_back();
      cmd += "}],Background -> LightGray,FrameStyle-> Dotted]";
    }
    if (cmd.back() == ',') {
      cmd.pop_back();
    }
    cmd += "}]]";
    return cmd;
  }

  std::unordered_set<LR_1_item>
  new_LR_1_item_set::expand_nonkernel_items(const CFG &cfg) const {
    std::unordered_set<LR_1_item> item_set;
    for (auto const &[head, lookahead_symbols] : nonkernel_items) {
      auto const &bodies = cfg.get_bodies(head);
      for (auto const &body : bodies) {
        if (body.empty()) {
          continue;
        }
        item_set.emplace(LR_0_item{head, body}, lookahead_symbols);
      }
    }
    return item_set;
  }
  const LR_1_item *new_LR_1_item_set::get_completed_item(
      CFG::terminal_type lookahead_symbol) const {
    for (auto const &completed_item : get_completed_items()) {
      if (completed_item.contain_lookahead_symbol(lookahead_symbol)) {
        return &completed_item;
      }
    }
    return nullptr;
  }

  /*
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
  */

} // namespace cyy::computation
