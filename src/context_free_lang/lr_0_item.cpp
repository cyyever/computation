/*!
 * \file lr_0_item.cpp
 */

#include "lr_0_item.hpp"

namespace cyy::computation {
  std::unordered_set<LR_0_item>
  LR_0_item_set::expand_nonkernel_items(const CFG &cfg) const {
    std::unordered_set<LR_0_item> item_set;
    for (auto const &head : nonkernel_items) {
      auto const &bodies = cfg.get_bodies(head);
      for (auto const &body : bodies) {
        if (body.empty()) {
          continue;
        }
        item_set.emplace(head, body);
      }
    }
    return item_set;
  }
  std::string LR_0_item::MMA_draw(const CFG &cfg) const {
    return get_production().MMA_draw(
        cfg.get_alphabet(), cfg.get_start_symbol() == get_head(),
        [&](size_t pos) {
          if (pos == dot_pos) {
            return "Style[\\[FilledSmallCircle],Red]";
          }
          return "";
        });
  }
  std::string LR_0_item_set::MMA_draw(const CFG &cfg) const {
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
} // namespace cyy::computation
