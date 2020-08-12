/*!
 * \file lr_0_item.cpp
 */

#include "lr_0_item.hpp"

namespace cyy::computation {
  std::string new_LR_0_item_set::MMA_draw(const ALPHABET &alphabet) const {
    std::string cmd = "Framed[TableForm[{";
    for (auto const &item : kernel_items) {
      cmd += item.MMA_draw(alphabet);
      cmd.push_back(',');
    }
    if (!nonkernel_items.empty()) {
      cmd += "Framed[TableForm[{";
      for (auto const &item : nonkernel_items) {
        cmd += grammar_symbol_type(item).MMA_draw(alphabet);
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
