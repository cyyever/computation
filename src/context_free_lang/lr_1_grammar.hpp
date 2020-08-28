/*!
 * \file lr_1_grammar.hpp
 *
 */

#pragma once

#include "dcfg.hpp"
#include "dk_1.hpp"
#include "lr_1_item.hpp"
#include "lr_grammar.hpp"

namespace cyy::computation {

  class LR_1_grammar : public LR_grammar {

  public:
    using LR_grammar::LR_grammar;
    using collection_type = DK_1_DFA::LR_1_item_set_collection_type;
    virtual std::pair<collection_type, goto_table_type>
    get_collection() const = 0;

    [[nodiscard]] bool
    parse(symbol_string_view view,
          const std::function<void(terminal_type)> &shift_callback,
          const std::function<void(const CFG_production &)> &reduction_callback)
        const override;

  private:
    bool DK_1_test(const collection_type &collection) const;
    void construct_parsing_table() const override;
  };
} // namespace cyy::computation
