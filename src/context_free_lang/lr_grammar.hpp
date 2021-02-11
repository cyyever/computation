/*!
 * \file lr_grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <functional>
#include <utility>

#include "cfg.hpp"

namespace cyy::computation {

  class LR_grammar : public CFG {

  public:
    using CFG::CFG;
    using state_type = finite_automaton::state_type;

    ~LR_grammar() override = default;

    parse_node_ptr get_parse_tree(symbol_string_view view) const;

    [[nodiscard]] bool virtual parse(
        symbol_string_view view,
        const std::function<void(terminal_type)> &shift_callback,
        const std::function<void(const CFG_production &)> &reduction_callback)
        const = 0;

  protected:
    using action_table_type =
        std::unordered_map<std::pair<state_type, terminal_type>,
                           std::variant<state_type, CFG_production, bool>>;
    using goto_table_type =
        std::unordered_map<std::pair<state_type, grammar_symbol_type>,
                           state_type>;

  private:
    virtual void construct_parsing_table() const = 0;
  };
} // namespace cyy::computation
