#include "context_free_lang/lr_0_item.hpp"
#include "formal_grammar/grammar_symbol.hpp"
#include "regular_lang/dfa.hpp"

namespace cyy::computation {
  class DK_DFA final {

    DK_DFA(
        DFA dfa_,
        std::unordered_map<grammar_symbol_type::nonterminal_type, symbol_type>
            nonterminal_to_state_,
        std::unordered_map<symbol_type, grammar_symbol_type::nonterminal_type>
            state_to_nonterminal_,
        lr_0_item_set_collection_type collection_);
    DK_DFA(const DK_DFA &) = default;
    DK_DFA &operator=(const DK_DFA &) = default;

    DK_DFA(DK_DFA &&) noexcept = default;
    DK_DFA &operator=(DK_DFA &&) noexcept = default;

    ~DK_DFA() = default;
    using goto_table_type =
        std::unordered_map<std::pair<DFA::state_type, grammar_symbol_type>,
                           DFA::state_type>;
    goto_table_type get_goto_table() const;

  private:
    DFA dfa;
    std::unordered_map<grammar_symbol_type::nonterminal_type, symbol_type>
        nonterminal_to_state;
    std::unordered_map<symbol_type, grammar_symbol_type::nonterminal_type>
        state_to_nonterminal;
    lr_0_item_set_collection_type collection;
  };
} // namespace cyy::computation
