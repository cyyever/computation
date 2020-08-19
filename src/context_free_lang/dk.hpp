/*!
 * \file dk.hpp
 */
#pragma once
#include <memory>

#include "cfg.hpp"
#include "context_free_lang/lr_0_item.hpp"
#include "formal_grammar/grammar_symbol.hpp"
#include "regular_lang/dfa.hpp"

namespace cyy::computation {
  class DK_DFA final {
  public:
    using state_type = DFA::state_type;
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
    goto_table_type get_goto_table() const;

    const LR_0_item_set &get_LR_0_item_set(state_type state) const;
    const auto &get_LR_0_item_set_collection() const { return collection; }
    std::string MMA_draw() const;

  private:
    std::shared_ptr<DFA> dfa_ptr;
    std::unordered_map<grammar_symbol_type::nonterminal_type, symbol_type>
        nonterminal_to_state;
    std::map<symbol_type, grammar_symbol_type::nonterminal_type>
        state_to_nonterminal;
    lr_0_item_set_collection_type collection;
  };
} // namespace cyy::computation
