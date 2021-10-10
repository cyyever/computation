/*!
 * \file dk.hpp
 */
#pragma once
#include <memory>

#include "cfg.hpp"
#include "alphabet/map_alphabet.hpp"
#include "regular_lang/dfa.hpp"

namespace cyy::computation {
  class DK_DFA_base {
  public:
    using state_type = DFA::state_type;
    DK_DFA_base(const CFG &cfg);
    DK_DFA_base(const DK_DFA_base &) = default;
    DK_DFA_base &operator=(const DK_DFA_base &) = default;

    DK_DFA_base(DK_DFA_base &&) noexcept = default;
    DK_DFA_base &operator=(DK_DFA_base &&) noexcept = default;

    ~DK_DFA_base() = default;
    auto const &get_dfa() const { return *dfa_ptr; }
    using goto_table_type =
        std::unordered_map<std::pair<state_type, grammar_symbol_type>,
                           state_type>;
    goto_table_type get_goto_table(bool skip_fail_state = false) const;

  protected:
    std::shared_ptr<DFA> dfa_ptr;
    std::shared_ptr<map_alphabet> alphabet_of_nonterminals;
  };
} // namespace cyy::computation
