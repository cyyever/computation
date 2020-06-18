#include "dcfg.hpp"

namespace cyy::computation {
  bool DCFG::DK_test() const {

    auto [dfa, state_to_LR_0_item_set] = get_DK();
    for (auto final_state : dfa.get_final_states()) {
      auto const &item_set = state_to_LR_0_item_set[final_state];
      if (item_set.get_nonkernel_items().size() != 1) {
        return false;
      }
      for (auto const &kernel_item : item_set.get_kernel_items()) {
        if (kernel_item.get_grammar_symbal().is_terminal()) {
          return false;
        }
      }
    }
    return true;
  }
} // namespace cyy::computation
