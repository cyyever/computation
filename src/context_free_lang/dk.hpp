/*!
 * \file dk.hpp
 */
#pragma once

#include "dk_base.hpp"
#include "lr_0_item.hpp"
#include "regular_lang/dfa.hpp"

namespace cyy::computation {
  class DK_DFA final : public DK_DFA_base {
  public:
    using lr_0_item_set_collection_type =
        std::unordered_map<state_type, LR_0_item_set>;
    DK_DFA(const CFG &cfg);
    DK_DFA(const DK_DFA &) = default;
    DK_DFA &operator=(const DK_DFA &) = default;

    DK_DFA(DK_DFA &&) noexcept = default;
    DK_DFA &operator=(DK_DFA &&) noexcept = default;

    ~DK_DFA() = default;

    const LR_0_item_set &get_LR_0_item_set(state_type state) const;
    const auto &get_LR_0_item_set_collection() const { return collection; }
    [[nodiscard]] std::string MMA_draw(const CFG &cfg) const;

  private:
    lr_0_item_set_collection_type collection;
  };
} // namespace cyy::computation
