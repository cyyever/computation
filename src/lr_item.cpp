/*!
 * \file lr_item.cpp
 *
 * \author cyy
 * \date 2018-04-21
 */

#include "lr_item.hpp"

namespace cyy::lang {

void LR_0_item_set ::add_kernel_item(const CFG &cfg, LR_0_item kernel_item) {
  std::set<CFG::nonterminal_type> tmp_nonkernel_items;
  if (kernel_item.dot_pos < kernel_item.production.second.size()) {
    auto const &symbol = kernel_item.production.second[kernel_item.dot_pos];
    if (auto ptr = std::get_if<CFG::nonterminal_type>(&symbol); ptr) {
      tmp_nonkernel_items.insert(*ptr);
    }
  }

  bool has_added = true;
  while (has_added) {
    has_added = false;

    for (const auto &nonkernel_item : tmp_nonkernel_items) {
      auto it = cfg.get_productions().find(nonkernel_item);

      for (auto const &body : it->second) {
        if (cfg.is_epsilon(body[0])) {
          kernel_items.emplace(LR_0_item{{it->first, body}, 1});
          continue;
        }

        if (auto ptr = std::get_if<CFG::nonterminal_type>(&body[0]); ptr) {
          if (tmp_nonkernel_items.insert(*ptr).second) {
            has_added = true;
          }
        }
      }
    }
  }
  kernel_items.emplace(std::move(kernel_item));
  nonkernel_items.merge(tmp_nonkernel_items);
}

void LR_1_item_set_ ::add_kernel_item(const CFG &cfg, LR_1_item kernel_item) {

  auto const &body = kernel_item.item.production.second;
  if (kernel_item.item.dot_pos >= body.size()) {
    kernel_items.emplace(std::move(kernel_item));
    return;
  }

  add_nonkernel_item(cfg,

                     CFG::grammar_symbol_string_view(
                         body.data() + kernel_item.item.dot_pos ,
                         body.size() - kernel_item.item.dot_pos ),
                     {kernel_item.lookahead});

  kernel_items.emplace(std::move(kernel_item));
  return;
}

void LR_1_item_set_::add_nonkernel_item(
    const CFG &cfg, CFG::grammar_symbol_string_view view,
    std::set<CFG::terminal_type> lookahead_set) {

  if (view.empty()) {
    return;
  }

  auto ptr = std::get_if<CFG::nonterminal_type>(&view[0]);

  if (!ptr) {
    return;
  }

  view.remove_prefix(1);
  auto real_lookahead_set = cfg.first(view);

  if (real_lookahead_set.erase(cfg.get_alphabet()->get_epsilon())) {
    real_lookahead_set.merge(lookahead_set);
  }


  decltype(real_lookahead_set) diff;

  std::set_difference(real_lookahead_set.begin(), real_lookahead_set.end(),
                      nonkernel_items[*ptr].begin(),
                      nonkernel_items[*ptr].end(),
                      std::inserter(diff, diff.begin()));

  if (diff.empty()) {
    return;
  }

  nonkernel_items[*ptr].merge(decltype(diff)(diff));

  auto it = cfg.get_productions().find(*ptr);

  for (auto const &new_body : it->second) {
    if(new_body.size()==1 && cfg.is_epsilon(new_body[0])) {

      LR_1_item new_item;
      new_item.item.dot_pos = 1;

      new_item.item.production.first = *ptr;
      new_item.item.production.second = new_body;
      for(auto const &lookahead:diff) {
      new_item.lookahead=lookahead;

      kernel_items.emplace(new_item);
      }
      continue;
    }


    add_nonkernel_item(cfg, {new_body.data(), new_body.size()}, diff);
  }

  return;
}
} // namespace cyy::lang
