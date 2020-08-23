/*!
 * \file dk.hpp
 */
#pragma once
#include <memory>

#include "cfg.hpp"
#include "context_free_lang/lr_0_item.hpp"
#include "lang/map_alphabet.hpp"
#include "regular_lang/dfa.hpp"

namespace cyy::computation {
  class DK_DFA final {
  public:
    using state_type = DFA::state_type;
    using lr_0_item_set_collection_type =
        std::unordered_map<DFA::state_type, LR_0_item_set>;
    DK_DFA(const CFG &cfg);
    DK_DFA(const DK_DFA &) = default;
    DK_DFA &operator=(const DK_DFA &) = default;

    DK_DFA(DK_DFA &&) noexcept = default;
    DK_DFA &operator=(DK_DFA &&) noexcept = default;

    ~DK_DFA() = default;
    auto const &get_dfa() const { return *dfa_ptr; }
    using goto_table_type =
        std::unordered_map<std::pair<DFA::state_type, grammar_symbol_type>,
                           DFA::state_type>;
    goto_table_type get_goto_table(bool skip_fail_state = false) const;

    const LR_0_item_set &get_LR_0_item_set(state_type state) const;
    const auto &get_LR_0_item_set_collection() const { return collection; }
    std::string MMA_draw(const CFG &cfg) const;

  private:
    std::shared_ptr<DFA> dfa_ptr;
    std::shared_ptr<map_alphabet> alphabet_of_nonterminals;
    lr_0_item_set_collection_type collection;
  };
} // namespace cyy::computation
