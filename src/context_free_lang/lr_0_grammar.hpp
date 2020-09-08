/*!
 * \file lr_0_grammar.hpp
 *
 */

#pragma once

#include "lr_0_item.hpp"
#include "lr_grammar.hpp"

namespace cyy::computation {

  class LR_0_grammar : public LR_grammar {

  public:
    using LR_grammar::LR_grammar;
    using collection_type = std::unordered_map<state_type, LR_0_item_set>;
    using goto_table_type =
        std::unordered_map<std::pair<state_type, grammar_symbol_type>,
                           state_type>;
    virtual std::pair<collection_type, goto_table_type>
    get_collection() const = 0;

    [[nodiscard]] bool
    parse(symbol_string_view view,
          const std::function<void(terminal_type)> &shift_callback,
          const std::function<void(const CFG_production &)> &reduction_callback)
        const override;

  protected:
    using reduction_table_type = std::unordered_map<state_type, CFG_production>;
    mutable reduction_table_type action_table;
    mutable goto_table_type goto_table;
    mutable std::unordered_set<state_type> final_states;

  private:
    void construct_parsing_table() const override;
  };
} // namespace cyy::computation
