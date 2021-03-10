/*!
 * \file dk.hpp
 */
#pragma once

#include "dk_base.hpp"
#include "lr_1_item.hpp"
#include "regular_lang/dfa.hpp"

namespace cyy::computation {
  class DK_1_DFA final : public DK_DFA_base {
  public:
    using LR_1_item_set_collection_type =
        std::unordered_map<DFA::state_type, LR_1_item_set>;
    DK_1_DFA(const CFG &cfg);
    DK_1_DFA(const DK_1_DFA &) = default;
    DK_1_DFA &operator=(const DK_1_DFA &) = default;

    DK_1_DFA(DK_1_DFA &&) noexcept = default;
    DK_1_DFA &operator=(DK_1_DFA &&) noexcept = default;

    ~DK_1_DFA() = default;

    const LR_1_item_set &get_LR_1_item_set(state_type state) const;
    const auto &get_LR_1_item_set_collection() const { return collection; }
    state_type get_reject_state() const;
    std::string [[nodiscard]] MMA_draw(const CFG &cfg) const;

  private:
    LR_1_item_set_collection_type collection;
  };
} // namespace cyy::computation
