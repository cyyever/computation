/*!
 * \file lr_grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <functional>
#include <map>
#include <utility>
#include <vector>

#include "cfg.hpp"
#include "../hash.hpp"

namespace cyy::computation {

  class LR_grammar : public CFG {

  public:
    using CFG::CFG;
    using state_type = uint64_t;

    virtual ~LR_grammar() = default;

    parse_node_ptr get_parse_tree(symbol_string_view view) const;

    [[nodiscard]] bool
    parse(symbol_string_view view,
          const std::function<void(terminal_type)> &shift_callback,
          const std::function<void(const CFG_production &)> &reduction_callback)
        const;

  private:
    virtual void construct_parsing_table() const = 0;

  protected:
    using action_table_type =
        std::unordered_map<std::pair<state_type, terminal_type>,
                 std::variant<state_type, CFG_production, bool>>;
    mutable action_table_type action_table;
    using goto_table_type =
        std::unordered_map<std::pair<state_type, nonterminal_type>, state_type>;
    mutable goto_table_type goto_table;
  };
} // namespace cyy::computation
